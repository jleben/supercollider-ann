#include "plugin.hpp"
#include "pool.hpp"
#include "fann_rt.hpp"

#include <cstring>
#include <algorithm>

#define IN_ANN_NUM 0
#define IN_WIN_SIZE 1
#define IN_SIG 2

// declare struct to hold unit generator state
struct AnnTime : public Unit
{
    int ann_i;
    int win_size;
    fann_type *buf;
};


// declare unit generator functions
static void AnnTime_Ctor(AnnTime* unit);
static void AnnTime_Dtor(AnnTime* unit);
static void AnnTime_idle(AnnTime *unit, int inNumSamples);
static void AnnTime_next(AnnTime *unit, int inNumSamples);

void defineAnnTimeUGen()
{
    DefineDtorUnit(AnnTime);
}

void AnnTime_Ctor(AnnTime* unit)
{
    unit->ann_i = -1;
    unit->buf = 0;

    int ann_i = IN0( IN_ANN_NUM );
    int win_size = IN0( IN_WIN_SIZE );

    if( win_size < 2 ) {
        Print("WARNING: AnnTime: window size must be at least 2");
        win_size = 2;
    }

    // init ANN input buffer

    fann_type* buf = (fann_type*) RTAlloc( unit->mWorld, sizeof(fann_type) * win_size);
    for(int i=0; i < win_size; ++i) buf[i] = 0.f;

    // set up the unit

    unit->ann_i = ann_i;
    unit->win_size = win_size;
    unit->buf = buf;

    SETCALC( AnnTime_next);

    // Calculate one sample of output

    AnnTime_next( unit, 1 );
}

void AnnTime_Dtor(AnnTime* unit)
{
    //Print("Destroying AnnTime UGen\n");
    RTFree( unit->mWorld, unit->buf );
}


void AnnTime_idle(AnnTime *unit, int numSamples)
{
    AnnDef *def = Ann_GetAnnDef<false>( unit->ann_i, unit->win_size, 1 );
    if(def) {
        SETCALC( AnnTime_next );
        AnnTime_next(unit, numSamples);
        return;
    }

    float *out = OUT(0);
    for( int i = 0; i < numSamples; ++i )
    {
        out[i] = 0.f;
    }
}

void AnnTime_next(AnnTime *unit, int numSamples)
{
    AnnDef *def = Ann_GetAnnDef<true>( unit->ann_i, unit->win_size, 1 );
    if(!def) {
        SETCALC( AnnTime_idle );
        AnnTime_idle(unit, numSamples);
        return;
    }

    struct fann *ann = def->_ann;
    fann_type *buf = unit->buf;
    int max_i = unit->win_size - 1;
    float *in = IN(IN_SIG);
    float *out = OUT(0);

    for(int i = 0; i < numSamples; ++i)
    {
        memmove( buf, buf+1, sizeof(fann_type) * (max_i) );
        buf[max_i] = in[i];

        fann_type *ann_out = fann_run( ann, buf );

        out[i] = ann_out[0];
    }
}
