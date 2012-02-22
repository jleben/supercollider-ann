#include "plugin.hpp"
#include "pool.hpp"
#include "fann_rt.hpp"

#include <cstring>

#define IN_ANN_NUM 0
#define IN_SIG 1
// negative offsets:
#define IN_TRAIN 3
#define IN_TD_C 2
#define IN_MSE 1

#define NUM_OTHER_IN 4

// declare struct to hold unit generator state
struct AnnAutoTrainer : public Unit
{
    int ann_i;
    struct fann_train_data *train_data;
    float target_mse;
    unsigned int max_td_c;

    bool train;
    float mse;
    int train_failure_c;
};


// declare unit generator functions
static void AnnAutoTrainer_Ctor(AnnAutoTrainer* unit);
static void AnnAutoTrainer_Dtor(AnnAutoTrainer* unit);
static void AnnAutoTrainer_failure(AnnAutoTrainer *unit, int inNumSamples);
static void AnnAutoTrainer_idle(AnnAutoTrainer *unit, int inNumSamples);
static void AnnAutoTrainer_acquire(AnnAutoTrainer *unit, int inNumSamples);
static void AnnAutoTrainer_train(AnnAutoTrainer *unit, int inNumSamples);

static unsigned int sigInCount(AnnAutoTrainer *unit)
{
    return unit->mNumInputs - NUM_OTHER_IN;
}

static unsigned int inTrain(AnnAutoTrainer *unit) { return unit->mNumInputs - IN_TRAIN; }
static unsigned int inTDC(AnnAutoTrainer *unit) { return unit->mNumInputs - IN_TD_C; }
static unsigned int inMSE(AnnAutoTrainer *unit) { return unit->mNumInputs - IN_MSE; }

static void startAcquiring( AnnAutoTrainer *unit )
{
    Print("Starting to acquire training sets\n");
    unit->train_data->num_data = 0;
    SETCALC( AnnAutoTrainer_acquire );
}

static void startTraining( AnnAutoTrainer *unit, struct fann *ann )
{
    Print("Starting training\n");
    unit->train_failure_c = 0;
    fann_randomize_weights( ann, 0.f, 1.f );
    SETCALC( AnnAutoTrainer_train );
}

static bool trainTrigger( AnnAutoTrainer *unit )
{
    int train_in = IN0(inTrain(unit));
    bool train = train_in >= 1;
    bool trig = train && !unit->train;
    unit->train = train;
    return trig;
}

void defineAnnAutoTrainerUGen()
{
    DefineDtorUnit(AnnAutoTrainer);
}

void AnnAutoTrainer_Ctor(AnnAutoTrainer* unit)
{
    unit->ann_i = -1;
    unit->target_mse = 0.f;
    unit->train_data = 0;
    unit->max_td_c = 0;
    unit->train = false;
    unit->mse = 0.f;
    unit->train_failure_c = 0;

    int ann_i = IN0( IN_ANN_NUM );
    unsigned int inc = sigInCount(unit);

    AnnDef *def = Ann_GetAnnDef<true>(ann_i, inc, inc);
    if( !def ) {
        Print("AnnAutoTrainer: Could not get ANN at index %i\n", ann_i);
        SETCALC(AnnAutoTrainer_failure);
        AnnAutoTrainer_failure( unit, 1 );
        return;
    }

    float mse = IN0(inMSE(unit));
    unsigned int td_c = (unsigned int)IN0(inTDC(unit));

    // init training data

    // create td with same amount of ins and outs
    struct fann_train_data *td = fann_rt_create_train( td_c, inc, inc, ft, unit->mWorld );
    // use td->num_data to denote how many td sets actually acquired
    td->num_data = 0;

    // set up the unit

    unit->ann_i = ann_i;
    unit->train_data = td;
    unit->target_mse = mse;
    unit->max_td_c = td_c;

    // start idle, to distribute CPU load
    SETCALC( AnnAutoTrainer_idle );
    AnnAutoTrainer_idle( unit, 1 );
}

void AnnAutoTrainer_Dtor(AnnAutoTrainer* unit)
{
    Print("Destroying AnnAutoTrainer UGen\n");
    fann_rt_destroy_train( unit->train_data, ft, unit->mWorld);
}

void AnnAutoTrainer_failure(AnnAutoTrainer *unit, int inNumSamples)
{
    OUT0(0) = unit->mse;
}

void AnnAutoTrainer_idle(AnnAutoTrainer *unit, int numSamples)
{
   if( trainTrigger(unit) )
        startAcquiring(unit);

   OUT0(0) = unit->mse;
}

void AnnAutoTrainer_acquire(AnnAutoTrainer *unit, int numSamples)
{
    unsigned int inc = sigInCount(unit);
    AnnDef *def = Ann_GetAnnDef<true>(unit->ann_i, inc, inc);
    if( !def ) {
        Print("AnnAutoTrainer: Could not get ANN at index %i\n", unit->ann_i);
        SETCALC(AnnAutoTrainer_failure);
        AnnAutoTrainer_failure( unit, 1 );
        return;
    }

    trainTrigger(unit);

    if( !unit->train ) {
        startTraining(unit, def->_ann);
        OUT0(0) = unit->mse;
        return;
    }

    struct fann_train_data *td = unit->train_data;
    int td_i = td->num_data;
    //int inc = td->num_input;
    fann_type *td_in = td->input[td_i];
    fann_type *td_out = td->output[td_i];

    for( int i = 0; i < inc; ++i )
        td_in[i] = IN0( IN_SIG + i );

    memcpy(td_out, td_in, inc * sizeof(fann_type));

    ++td_i;
    td->num_data = td_i;

    if( td_i >= unit->max_td_c ) {
        startTraining(unit, def->_ann);
    }

    OUT0(0) = unit->mse;
}

void AnnAutoTrainer_train(AnnAutoTrainer *unit, int numSamples)
{
    struct fann_train_data *td = unit->train_data;
    int inc = td->num_input;
    float tmse = unit->target_mse;
    float *out = OUT(0);

    if( trainTrigger(unit) ) {
        unit->train = true;
        // begin acquiring
        startAcquiring(unit);
        out[0] = unit->mse;
        return;
    }

    if( td->num_data < 1 ) {
        Print("No training data.");
        SETCALC( AnnAutoTrainer_idle );
        out[0] = unit->mse;
        return;
    }

    AnnDef *def = Ann_GetAnnDef<true>( unit->ann_i, inc, inc );
    if(!def) {
        Print("Failure.");
        SETCALC( AnnAutoTrainer_failure );
        AnnAutoTrainer_failure(unit, numSamples);
        return;
    }

    struct fann *ann = def->_ann;

    float min_mse = fann_train_epoch(ann, td);

    int epoch_c = 5;

    while(epoch_c-- && (min_mse > tmse) && (unit->train_failure_c < 1000) ) {
        float mse = fann_train_epoch(ann, td);

        if( mse >= min_mse )
            ++unit->train_failure_c;
        else
            min_mse = mse;
    }

    float real_mse = fann_get_MSE(ann);

    //Print("MSE = %f, failure = %i\n", real_mse, unit->train_failure_c);

    if( real_mse <= tmse || unit->train_failure_c >= 1000 )
    {
        Print("Stopped training at %f, min %f\n", real_mse, min_mse);
        SETCALC( AnnAutoTrainer_idle );
    }

    out[0] = real_mse;

    unit->mse = real_mse;
}

