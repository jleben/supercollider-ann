#include "plugin.hpp"
#include "pool.hpp"
#include "fann_rt.hpp"

#include <cstring>

#define IN_ANN_NUM 0
#define IN_SIG 1
#define NUM_OTHER_IN 2

// declare struct to hold unit generator state
struct AnnBasic : public Unit
{
    int ann_i;
    fann_type *inputs;
};


// declare unit generator functions
static void AnnBasic_Ctor(AnnBasic* unit);
static void AnnBasic_Dtor(AnnBasic* unit);
static void AnnBasic_idle(AnnBasic *unit, int inNumSamples);
static void AnnBasic_next(AnnBasic *unit, int inNumSamples);

static unsigned int sigInCount(AnnBasic *unit)
{
    return unit->mNumInputs - NUM_OTHER_IN;
}

static unsigned int sigOutCount(AnnBasic *unit)
{
    return unit->mNumOutputs;
}

void defineAnnBasicUGen()
{
    DefineDtorUnit(AnnBasic);
}

void AnnBasic_Ctor(AnnBasic* unit)
{
    unit->ann_i = -1;
    unit->inputs = 0;

    int ann_i = IN0( IN_ANN_NUM );
    unsigned int inc = sigInCount(unit);
    unsigned int outc = sigOutCount(unit);

    // init ANN input buffer

    fann_type* ann_ins = (fann_type*) RTAlloc( unit->mWorld, sizeof(fann_type) * inc);
    for(int i=0; i < inc; ++i) ann_ins[i] = 0.f;

    // set up the unit

    unit->ann_i = ann_i;
    unit->inputs = ann_ins;
    SETCALC( AnnBasic_next);

    // Calculate one sample of output

    AnnBasic_next( unit, 1 );
    //AnnBasic_idle( unit, 1 );
}

void AnnBasic_Dtor(AnnBasic* unit)
{
    Print("Destroying AnnBasic UGen\n");
    //fann_rt_destroy( unit->ann, ft, unit->mWorld );
    RTFree( unit->mWorld, unit->inputs );
}


void AnnBasic_idle(AnnBasic *unit, int numSamples)
{
    AnnDef *def = Ann_GetAnnDef<false>( unit->ann_i, sigInCount(unit), sigOutCount(unit) );
    if(def) {
        SETCALC( AnnBasic_next );
        AnnBasic_next(unit, numSamples);
        return;
    }

    float *out = OUT(0);
    out[0] = 0.f;
}

void AnnBasic_next(AnnBasic *unit, int numSamples)
{
    unsigned int in_c = sigInCount(unit);
    unsigned int out_c = sigOutCount(unit);

    AnnDef *def = Ann_GetAnnDef<true>( unit->ann_i, sigInCount(unit), sigOutCount(unit) );
    if(!def) {
        SETCALC( AnnBasic_idle );
        AnnBasic_idle(unit, numSamples);
        return;
    }

    struct fann *ann = def->_ann;
    float *in = unit->inputs;

    for( int i = 0; i < in_c; ++i )
    {
        in[i] = IN0(IN_SIG + i);
    }

    fann_type *out = fann_run( ann, in );

    for( int i = 0; i < out_c; ++i )
    {
        OUT0(i) = out[i];
    }
}
