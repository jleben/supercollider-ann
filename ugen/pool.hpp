#ifndef ANN_POOL_H
#define ANN_POOL_H

#include "plugin.hpp"

#include <floatfann.h>
#include <string.h>

struct AnnDef {
    char *_name;
    struct fann *_ann;
    bool training;

    AnnDef( const char *name, struct fann *ann ):
        _name(strdup(name)), _ann(ann), training(false) {}

    ~AnnDef() { free(_name); }
};

void initPool();

AnnDef * getAnnDef(unsigned int i);

template<bool msg> AnnDef *Ann_GetAnnDef( int i, unsigned int inc, unsigned int outc )
{
    AnnDef *def = getAnnDef(i);
    if(!def) {
        if(msg) Print("Could not get ANN at index %i\n", i);
        return 0;
    }
    struct fann *ann = def->_ann;
    if( inc != fann_get_num_input(ann) ) {
        if(msg) Print( "Error: Input count mismatch ugen: %i / ann: %i\n", inc, fann_get_num_input(ann) );
        return 0;
    }
    if( outc != fann_get_num_output(ann) ) {
        if(msg) Print( "Error: Output count mismatch ugen: %i / ann: %i\n", outc, fann_get_num_output(ann) );
        return 0;
    }
    return def;
}

#endif // ANN_POOL_H
