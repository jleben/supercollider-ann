#include <floatfann.h>

#if defined(FIXEDFANN)

# warning Real-time features of Fann can only be compiled in floating point version.

#elif !defined(__fann_rt_h__)

# warning Using Real-time feature header.

#define __fann_rt_h__

#include <SC_InterfaceTable.h>

extern "C"
{

struct fann_rt
{
    /* The type of error that last occured. */
    enum fann_errno_enum errno_f;

    /* Where to log error messages. */
    FILE *error_log;

    /* A string representation of the last error. */
    char *errstr;

    /* the connection rate of the network
     * between 0 and 1, 1 meaning fully connected
     */
    float connection_rate;

    /* is 1 if shortcut connections are used in the ann otherwise 0
     * Shortcut connections are connections that skip layers.
     * A fully connected ann with shortcut connections are a ann where
     * neurons have connections to all neurons in all later layers.
     */
    enum fann_nettype_enum network_type;

    /* pointer to the first layer (input layer) in an array af all the layers,
     * including the input and outputlayers
     */
    struct fann_layer *first_layer;

    /* pointer to the layer past the last layer in an array af all the layers,
     * including the input and outputlayers
     */
    struct fann_layer *last_layer;

    /* Total number of neurons.
     * very usefull, because the actual neurons are allocated in one long array
     */
    unsigned int total_neurons;

    /* Number of input neurons (not calculating bias) */
    unsigned int num_input;

    /* Number of output neurons (not calculating bias) */
    unsigned int num_output;

    /* The weight array */
    fann_type *weights;

    /* The connection array */
    struct fann_neuron **connections;

    /* Total number of connections.
     * very usefull, because the actual connections
     * are allocated in one long array
     */
    unsigned int total_connections;

    /* used to store outputs in */
    fann_type *output;

    /* A pointer to user defined data. (default NULL)
    */
    void *user_data;
};

} // extern "C"

struct fann_rt *fann_rt_create( struct fann *ann, InterfaceTable *ft, World *w );

void fann_rt_destroy( struct fann_rt *ann, InterfaceTable *ft, World *w );

fann_type * fann_rt_run(struct fann_rt * ann, fann_type * input);

struct fann_train_data * fann_rt_create_train(
    unsigned int num_data, unsigned int num_input, unsigned int num_output,
    InterfaceTable *ft, World *w
);

void fann_rt_destroy_train( struct fann_train_data *, InterfaceTable *ft, World *w );

#endif // __fann_rt_h__
