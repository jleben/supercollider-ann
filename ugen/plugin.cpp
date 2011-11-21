#include "plugin.hpp"
#include "pool.hpp"

PluginLoad(Ann)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable;

    initPool();
    defineAnnUGen();
    //defineAnnBasicUGen();
}

