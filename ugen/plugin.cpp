#include "plugin.hpp"
#include "pool.hpp"

// InterfaceTable contains pointers to functions in the host (server).
InterfaceTable *ft;

PluginLoad(Ann)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable;

    initPool();
    defineAnnBasicUGen();
    defineAnnAutoTrainerUGen();
}

