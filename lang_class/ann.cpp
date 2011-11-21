#include <SC_Lang_API.h>

using namespace SC::Lang;
using namespace std;

static void initPrimitives();
static void startup( const PluginIntf & );

///////////////////////////////////////////////////////////////////////////////////////////////////

DeclareLanguagePlugin( "Ann", "SuperCollider interface to FANN "
                              "- Fast Artifical Neural Network library",
                       &initPrimitives, &startup, 0 );

///////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#include <PyrPrimitive.h>
#include <VMGlobals.h>
#include <PyrObject.h>
#include <SCBase.h>
#include <PyrKernel.h>
#endif

#include <fann.h>

#include <assert.h>
#include <map>
#include <iostream>
#include <cstring>
#include <cstdio>

typedef map<Symbol, fann_activationfunc_enum> ActFuncMap;
typedef pair<Symbol, fann_activationfunc_enum> ActFuncId;

#define GET_ANN_DATA(obj) static_cast<AnnData*>( obj[1].asRawPointer() );

struct AnnData {
  AnnData() : net(0), trainData(0), runData(0) {}
  struct fann *net;
  struct fann_train_data *trainData;
  fann_type *runData;
};

int Ann_GetActivationFuncNames( State & );
int Ann_Create( State &state );
int Ann_Load( State &state );
int Ann_InputCount( State &state );
int Ann_OutputCount( State &state );
int Ann_SetActivationFunc( State &state );
int Ann_SetTrainingData( State &state );
int Ann_Train( State &state );
int Ann_Run( State &state );
int Ann_Save( State &state );
int Ann_Finalize( State &, Object & );

static Class arrayClass;
static Class stringClass;
static Symbol sym_all;
static Symbol sym_hidden;
static Symbol sym_output;
static Sys *lang;
static ActFuncMap actFuncMap;

#define MAP_ACT_FUNC_NAME( name ) { \
    Symbol sym = lang->getSymbol( FANN_ACTIVATIONFUNC_NAMES[name] ); \
    actFuncMap.insert( ActFuncId(sym, name) ); \
    }

static void startup( const PluginIntf &intf ) {
    lang = new Sys(intf.system());
#if 0
    // FIXME: doesn't work, sizeof() gives 144
    int actFuncCount = sizeof(FANN_ACTIVATIONFUNC_NAMES);
    cout << "act func count = " << actFuncCount << endl;
    for( int i = 0; i < actFuncCount; ++i ) {
        const char *name = FANN_ACTIVATIONFUNC_NAMES[i];
        if( !name ) continue;
        Symbol sym = lang->getSymbol( name );
        actFuncMap.insert( ActFuncId(sym, i) );
    }
#endif
    MAP_ACT_FUNC_NAME( FANN_LINEAR );
    MAP_ACT_FUNC_NAME( FANN_THRESHOLD );
    MAP_ACT_FUNC_NAME( FANN_THRESHOLD_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_SIGMOID );
    MAP_ACT_FUNC_NAME( FANN_SIGMOID_STEPWISE );
    MAP_ACT_FUNC_NAME( FANN_SIGMOID_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_GAUSSIAN );
    MAP_ACT_FUNC_NAME( FANN_GAUSSIAN_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_ELLIOT );
    MAP_ACT_FUNC_NAME( FANN_ELLIOT_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_LINEAR_PIECE );
    MAP_ACT_FUNC_NAME( FANN_LINEAR_PIECE_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_SIN_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_COS_SYMMETRIC );
    MAP_ACT_FUNC_NAME( FANN_SIN );
    MAP_ACT_FUNC_NAME( FANN_COS );
}

static void initPrimitives()
{
  printf("Initializing Ann primitives.\n");
  lang->definePrimitive<&Ann_GetActivationFuncNames>( "_Ann_GetActivationFuncNames", 0, 0 );
  lang->definePrimitive<&Ann_Create>( "_Ann_Create", 1, 0 );
  lang->definePrimitive<&Ann_Load>( "_Ann_Load", 1, 0 );
  lang->definePrimitive<&Ann_InputCount>( "_Ann_InputCount", 0, 0 );
  lang->definePrimitive<&Ann_OutputCount>( "_Ann_OutputCount", 0, 0 );
  lang->definePrimitive<&Ann_SetActivationFunc>( "_Ann_SetActivationFunc", 2, 0 );
  lang->definePrimitive<&Ann_SetTrainingData>( "_Ann_SetTrainingData", 1, 0 );
  lang->definePrimitive<&Ann_Train>( "_Ann_Train", 0, 0 );
  lang->definePrimitive<&Ann_Run>( "_Ann_Run", 2, 0 );
  lang->definePrimitive<&Ann_Save>( "_Ann_Save", 1, 0 );

  arrayClass = Class( lang->getSymbol("Array") );
  stringClass = Class( lang->getSymbol("String") );
  sym_all = lang->getSymbol("all");
  sym_hidden = lang->getSymbol("hidden");
  sym_output = lang->getSymbol("output");
}

int Ann_GetActivationFuncNames( State &s )
{
    int count = actFuncMap.size();
    Object a( lang->newArray( s, count, 0, true ) );
    s.receiver() = a;

    a.setSize( count );

    ActFuncMap::iterator it;
    int i;
    for( i = 0, it = actFuncMap.begin(); it != actFuncMap.end(); ++i, ++it )
        a[i] = it->first;

    return errNone;
}

int Ann_Create( State &state )
{
  Object obj = state.receiver().asObject();

  Slot sLayers = state[0];
  if( !sLayers.isKindOf( arrayClass ) ) return errWrongType;
  Object layers = sLayers.asObject();

  if( layers.size() < 2 ) {
    printf("Ann: there must be at least 2 layers (input and output)\n");
    return errFailed;
  }

  unsigned int *layerv = new unsigned int [layers.size()];

  int err = errNone;

  for( int i=0; i<layers.size(); ++i ){
    bool ok;
    int val = layers[i].toInt(&ok);

    if(!ok) {
      printf("Ann: wrong type in structure Array at index %i.\n", i);
      delete[] layerv;
      return errFailed;
    }
    if( val < 1 ) {
      printf("Ann: layer %i: a layer must have at least one node.\n", i);
      delete[] layerv;
      return errFailed;
    }
    layerv[i] = val;
  }

  struct fann *net = fann_create_standard_array( layers.size(), layerv );

  delete[] layerv;

  AnnData *d = new AnnData;
  d->net = net;
  d->runData = new fann_type[fann_get_num_input(net)];

  obj[1].setRawPointer( d );

  lang->installFinalizer<Ann_Finalize>( state, obj, 0 );

  return errNone;
}

int Ann_Load( State &state )
{
    Object obj( state.receiver().asObject());

    // assert it is new, e.g. finalizer and data don't exist
    assert( obj[0].isNil() && obj[1].isNil() );

    String str( state[0].toString() );

    if( !str.isValid() ) return errWrongType;

    std::string std_str( str.stdString() );
    printf( "Loading Ann from: %s\n", std_str.c_str() );
    struct fann *net = fann_create_from_file( std_str.c_str() );
    if(!net) return errFailed;

    AnnData *d = new AnnData;
    d->net = net;
    d->runData = new fann_type[fann_get_num_input(net)];

    obj[1].setRawPointer( d );

    lang->installFinalizer<Ann_Finalize>( state, obj, 0 );

    return errNone;
}

int Ann_InputCount( State &state )
{
    AnnData *d = GET_ANN_DATA(state.receiver().asObject());
    state.receiver() = (int)fann_get_num_input(d->net);
    return errNone;
}

int Ann_OutputCount( State &state )
{
    AnnData *d = GET_ANN_DATA(state.receiver().asObject());
    state.receiver() = (int)fann_get_num_output(d->net);
    return errNone;
}

int Ann_SetActivationFunc( State &state )
{
    AnnData *d = GET_ANN_DATA(state.receiver().asObject());

    Symbol sym = state[0].toSymbol();
    if( !sym.isValid() ) return errWrongType;

    Slot layers = state[1];
    if( !layers.isSymbol() ) return errWrongType;

    ActFuncMap::iterator it = actFuncMap.find(sym);
    if( it == actFuncMap.end() ) return errFailed;

    sym = layers.asSymbol();
    if (sym == sym_all) {
        fann_set_activation_function_hidden( d->net, it->second );
        fann_set_activation_function_output( d->net, it->second );
    }
    else if (sym == sym_hidden) {
        fann_set_activation_function_hidden( d->net, it->second );
    }
    else if (sym == sym_output) {
        fann_set_activation_function_output( d->net, it->second );
    }
    else {
        return errFailed;
    }

    return errNone;
}

int Ann_Finalize( State &state, Object &obj  )
{
  AnnData *d = GET_ANN_DATA(obj);
  fann_destroy( d->net );
  fann_destroy_train( d->trainData );
  delete[] d->runData;
}

static Object gTrainDataArray;

static void Ann_PutTrainData ( unsigned int i, unsigned int inc, unsigned int outc, fann_type *inv, fann_type *outv )
{
  Object &data = gTrainDataArray;
  assert( i < data.size() );

  Slot s = data[i];
  if( !s.isObject() || !s.isKindOf(arrayClass) ) {
    printf("Ann: training data set @%i not an Array!\n", i);
    return;
  }
  Object set = s.asObject();
  if( set.size() < inc + outc ) {
    printf("Ann: training data set @%i has too few elements\n", i);
    return;
  }

  //post("INPUTS: ");

  int j;
  for( j=0; j<inc; j++ ) {
    bool ok;
    double val = set[j].toDouble(&ok);
    if( !ok ) {
      printf("Ann: wrong type in input training data @%i@%i\n", i, j);
      continue;
    }
    inv[j] = val;
    //post("%f ", val );
  }

  //post("\nOUTPUTS:");

  for( ; j < inc + outc; j++ ) {
    bool ok;
    double val = set[j].toDouble(&ok);
    if( !ok ) {
      printf("Ann: wrong type in input training data @%i@%i\n", i, j);
      continue;
    }
    outv[j-inc] = val;
    //post("%f ", val );
  }

  //post("\n");

#if 0
  // INPUTS
  PyrObject *in;
  if( !isKindOfSlot(set->slots, class_array) ) {
    error("Ann: input training data @%i not an Array!\n", i);
  }
  else if( (in = slotRawObject(set->slots))->size < inc ) {
    error("Ann: input training data @%i has less values than %i!\n", i, inc);
  }
  else {
    for( int j=0; j < inc; ++j ) {
      double val;
      int err = slotDoubleVal( in->slots+j, &val );
      if( err ) {
        error("Ann: wrong type in input training data @%i@%i\n", i, j);
        continue;
      }
      inv[j] = val;
      post("%f ", val );
    }
  }

  post("\nOUTPUTS:");
  // OUTPUTS
  PyrObject *out;
  if( !isKindOfSlot(set->slots+1, class_array) ) {
    error("Ann: output training data @%i not an Array!\n", i);
  }
  else if( (out = slotRawObject(set->slots+1))->size < outc ) {
    error("Ann: output training data @%i has less values than %i!\n", i, inc);
  }
  else {
    for( int j=0; j < outc; ++j ) {
      double val;
      int err = slotDoubleVal( out->slots+j, &val );
      if( err ) {
        error("Ann: wrong type in output training data @%i@%i\n", i, j);
        continue;
      }
      outv[j] = val;
      post("%f ", val );
    }
  }
  post("\n");
#endif
}

int Ann_SetTrainingData( State &state )
{
  Slot sData = state[0];
  if( !sData.isKindOf( arrayClass ) ) return errWrongType;;
  Object dataSets = sData.toObject();

  AnnData *d = GET_ANN_DATA(state.receiver().asObject());

  fann_destroy_train( d->trainData );

  int count = dataSets.size();
  gTrainDataArray = dataSets;
  struct fann_train_data *td =
    fann_create_train_from_callback( dataSets.size(),
                                     fann_get_num_input(d->net),
                                     fann_get_num_output(d->net),
                                     &Ann_PutTrainData );

  d->trainData = td;

  return errNone;
}

int Ann_Train( State &state )
{
  AnnData *d = GET_ANN_DATA(state.receiver().asObject());
  if( !d->trainData ) {
    printf("Ann: no training data has been set\n");
    return errFailed;
  }
  float mse = fann_train_epoch( d->net, d->trainData );
  state.receiver() = mse;
  return errNone;
}

int Ann_Run( State &state )
{
  Slot sIn = state[0];
  Slot sOut = state[1];

  if( !sIn.isKindOf( arrayClass ) ) {
    printf("Ann: input argument not an Array\n");
    return errWrongType;
  }
  if( !sOut.isKindOf( arrayClass ) ) {
    printf("Ann: output argument not an Array\n");
    return errWrongType;
  }


  Object in = sIn.asObject();
  Object out = sOut.asObject();

  AnnData *d = GET_ANN_DATA(state.receiver().asObject());

  if( in.size() < fann_get_num_input(d->net) ) {
    printf("Ann: input data contains too few elements\n");
    return errFailed;
  }
#if 0
  // TODO!!!
  if( ARRAYMAXINDEXSIZE(out) < fann_get_num_output(d->net) ) {
    printf("Ann: output Array is too small\n");
    return errFailed;
  }
#endif

  for( int i = 0; i < in.size(); ++i ) {
    bool ok;
    float val = in[i].toFloat(&ok);
    if( !ok ) {
      printf("Ann: wrong type in input data\n");
      return errFailed;
    }
    d->runData[i] = val;
  }

  fann_type *result = fann_run( d->net, d->runData );

  out.setSize( fann_get_num_output(d->net) );
  for( int i = 0; i < out.size(); ++i ) {
    out[i] = result[i];
  }

  state.receiver() = out;

  return errNone;
}

int Ann_Save( State &state )
{
  Slot sFile = state[0];
  String str = sFile.toString();
  if( !sFile.isValid() ) return errWrongType;

  std::string std_str( str.stdString() );
  printf( "Saving Ann to: %s\n", std_str.c_str() );

  AnnData *d = GET_ANN_DATA(state.receiver().asObject());
  if( fann_save( d->net, std_str.c_str() ) ) return errFailed;

  return errNone;
}
