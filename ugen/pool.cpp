#include "pool.hpp"

#include <SC_PlugIn.h>

// data structs

struct LoadAnnData {
  int id;
  char *filename;
  AnnDef *annDef;
};

// global data

extern InterfaceTable *ft;

const int kAnnDefCount = 20;
AnnDef * annDefs[kAnnDefCount];

// function declarations

static void loadAnnCmd(World *, void * userData, struct sc_msg_iter *args, void *replyAddr);
static bool loadAnnCmd2(World *, void * userData );
static bool loadAnnCmd3(World *, void * userData );
static void loadAnnCleanup(World *, void * userData);


static struct fann * loadFann( const char *filename );
//int findAnnDef( const char *name );


// function implementation

void initPool() {
  // clear the AnnDef array
  for( int i = 0; i < kAnnDefCount; ++i ) {
    annDefs[i] = 0;
  }
  // define the load command
  DefinePlugInCmd("loadAnn", loadAnnCmd, 0 );
}

static void loadAnnCmd(World *w, void* userData, struct sc_msg_iter *args, void *replyAddr)
{
  Print( "loadAnnFunc\n" );

  // parse args
  int id = args->geti(-1);
  if( id < 0 || id > kAnnDefCount ) return;

  const char *filename = args->gets( 0 );
  if( !filename || strlen(filename) < 1 ) return;

  // alloc and init cmd data struct
  LoadAnnData *data = (LoadAnnData*) RTAlloc( w, sizeof(LoadAnnData) );
  data->id = id;
  data->filename = (char*) RTAlloc( w, strlen(filename) + 1 );
  strcpy( data->filename, filename );
  data->annDef = annDefs[id];

  // clear the affected AnnDef
  annDefs[id] = 0;

  // execute the command
  DoAsynchronousCommand(w, replyAddr, "loadAnn", (void*) data,
                        (AsyncStageFn)&loadAnnCmd2,
                        (AsyncStageFn)&loadAnnCmd3,
                        (AsyncStageFn)0,
                        loadAnnCleanup,
                        0, 0);
}

static bool loadAnnCmd2(World *, void * userData )
{
  LoadAnnData *data = static_cast<LoadAnnData*>(userData);

  Print( "Loading Fann file: %s\n", data->filename );

  delete data->annDef;
  data->annDef = 0;

  struct fann *ann = loadFann( data->filename );
  if( !ann ) return false;
  data->annDef = new AnnDef( "", ann );

  return true;
}

static bool loadAnnCmd3(World *, void * userData )
{
  LoadAnnData *data = static_cast<LoadAnnData*>(userData);
  Print( "Storing AnnDef into index %i\n", data->id );
  annDefs[data->id] = data->annDef;

  return true;
}

static void loadAnnCleanup(World *w, void * userData)
{
  Print( "Cleaning up load cmd.\n", static_cast<LoadAnnData*>(userData)->id );
  RTFree( w, userData );
}

static struct fann * loadFann( const char *filename )
{
  struct fann *ann = fann_create_from_file( filename );
  if( ann ) Print( "Ann: Successfully loaded ANN from file: %s\n", filename );
  else Print( "Ann: Error loading ANN from file: %s\n", filename );
  return ann;
}

#if 0
int findAnnDef( const char *name )
{
  for( int i = 0; i < kAnnDefCount; ++i ) {
    if( !strcmp( annDefs[i]->_name, name ) ) return i;
  }
  return -1;
}

static void removeAnnDef( int i )
{
  if( i < 0 || i >= annDefcount ) return;
  AnnDefVector::Iterator it = annDefs.begin() + i;
  delete *it;
  annDefs.remove(it);
}

static void removeAnnDef( const char *name )
{
  AnnDefVector::Iterator it;
  for( it = annDefs.begin(); it != annDefs.end(); ++it ) {
    AnnDef *def = *it;
    if( !strcmp( def->_name, name ) ) {
      delete def;
      annDefs.remove(it);
      return;
    }
  }
}



static void addAnnDef( const char *name, const char *filename )
{
  struct fann *ann = loadFann( filename );
  if( !ann ) return;
  AnnDef *def = new AnnDef( name, ann );
  annDefs.push_back( def );
}

static void replaceAnnDef( int i, const char *name, const char *filename )
{
  if( i < 0 || i >= annDefs.size() ) {
    Print( "Ann: Failed to replace ANN at index %i. Index out of range.\n", i );
    return;
  }
  AnnDef *def = *(annDefs.begin() + i);
  if( name ) {
    free( def->_name );
    def->_name = strcpy(name);
  }
  if( filename ) {
    struct fann *ann = loadFann( filename );
    if( ann ){
      fann_destroy( def->_ann );
      def->_ann = ann;
    }
  }
}
#endif
