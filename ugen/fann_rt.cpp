#include "fann_rt.hpp"

#ifndef FIXEDFANN

# warning Using Real-time feature source.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>

static struct fann_rt * fann_rt_alloc( InterfaceTable *ft, World *w );

void fann_rt_destroy( struct fann_rt *ann, InterfaceTable *ft, World *w );

void fann_rt_destroy_train( struct fann_train_data *, InterfaceTable *ft, World *w );

struct fann_rt *fann_rt_create( struct fann *ann, InterfaceTable *ft, World *w )
{
#if 0
  Print("fann_rt_create\n");
#endif

  struct fann_rt *new_ann = fann_rt_alloc( ft, w );
  if( new_ann == NULL ) {
    Print("Ann: could not allocate fann_rt structure.\n");
    return NULL;
  }
  //Print("allocated fann_rt\n");

  new_ann->errno_f = FANN_E_NO_ERROR;
  new_ann->error_log = fann_default_error_log;
  new_ann->errstr = NULL;

  new_ann->connection_rate = ann->connection_rate;
  new_ann->network_type = ann->network_type;
  new_ann->total_neurons = ann->total_neurons;
  new_ann->total_connections = ann->total_connections;
  new_ann->num_input = ann->num_input;
  new_ann->num_output = ann->num_output;

  // allocate layers

  int num_layers = ann->last_layer - ann->first_layer;
  new_ann->first_layer = (struct fann_layer*) RTAlloc( w, sizeof( struct fann_layer ) * num_layers );
  new_ann->last_layer = new_ann->first_layer + num_layers;

  if( new_ann->first_layer == NULL ) {
    Print("Ann: Could not allocate layers.\n");
    fann_rt_destroy( new_ann, ft, w );
    return NULL;
  }
  //Print("allocated layers\n");

  // set first neuron so fann_rt_destroy does not try to free an invalid address
  new_ann->first_layer->first_neuron = 0;

  // allocate neurons
  struct fann_neuron *neurons;
  neurons = (struct fann_neuron *) RTAlloc( w, sizeof(struct fann_neuron) * new_ann->total_neurons );

  if(neurons == NULL)
  {
    Print("Ann: Could not allocate neurons.\n");
    fann_rt_destroy( new_ann, ft, w );
    return NULL;
  }
  //Print("allocated %u neurons\n", new_ann->total_neurons);

  // bind neurons into layers

  struct fann_layer *layer1 = ann->first_layer;
  struct fann_layer *layer2 = new_ann->first_layer;
  for( ; layer2 != new_ann->last_layer; ++layer1, ++layer2 )
  {
      unsigned int num_neurons = layer1->last_neuron - layer1->first_neuron;
      layer2->first_neuron = neurons;
      neurons += num_neurons;
      layer2->last_neuron = neurons;
  }
#if 0
  Print("wrote %u neurons into layers\n",
        (new_ann->last_layer-1)->last_neuron - new_ann->first_layer->first_neuron);
#endif

  // allocate outputs

  new_ann->output = (fann_type *) RTAlloc( w, sizeof(fann_type) * new_ann->num_output );
  if(ann->output == NULL)
  {
    Print("Ann: Could not allocate outputs.\n");
    fann_rt_destroy( new_ann, ft, w );
    return NULL;
  }
  //Print("allocated outputs\n");

  // copy neuron data
  // use last value of neurons - it should be just past all allocated neurons

  struct fann_neuron *neuron_it1 = ann->first_layer->first_neuron;
  struct fann_neuron *neuron_it2 = new_ann->first_layer->first_neuron;
  for( ;  neuron_it2 != neurons ; ++neuron_it1, ++neuron_it2 )
  {
    neuron_it2->activation_function = neuron_it1->activation_function;
    neuron_it2->activation_steepness = neuron_it1->activation_steepness;
    neuron_it2->first_con = neuron_it1->first_con;
    neuron_it2->last_con = neuron_it1->last_con;
#if 0
    Print("%i neuron connections: %i - %i\n",
          neuron_it2->last_con - neuron_it2->first_con,
          neuron_it2->first_con,
          neuron_it2->last_con);
#endif
    neuron_it2->sum = 0;
    neuron_it2->value = 0;
  }
  //Print("initialized neurons\n");

  // allocate weights and connections

  new_ann->weights = (fann_type *) RTAlloc( w, sizeof(fann_type) * new_ann->total_connections );
  if(new_ann->weights == NULL)
  {
    Print("Ann: Could not allocate weights\n");
    fann_rt_destroy( new_ann, ft, w );
    return NULL;
  }

  new_ann->connections =
      (struct fann_neuron **) RTAlloc( w, sizeof(struct fann_neuron *) * new_ann->total_connections );
  if(new_ann->connections == NULL)
  {
    Print("Ann: Could not allocate connections\n");
    fann_rt_destroy( new_ann, ft, w );
    return NULL;
  }

  // copy over weights and connections

  neuron_it1 = ann->first_layer->first_neuron;
  neuron_it2 = new_ann->first_layer->first_neuron;
  for( int i = 0; i < new_ann->total_connections; ++i )
  {
    new_ann->weights[i] = ann->weights[i];
    unsigned int offset = ann->connections[i] - neuron_it1;
#if 0
    Print("connection offset = %u\n", offset);
#endif
    new_ann->connections[i] = neuron_it2 + offset;//( ann->connections[i] - neuron_it1 );
  }

#if 0
  Print("fann_rt_create finished successfully.\n");
#endif

  return new_ann;
}

struct fann_rt * fann_rt_alloc( InterfaceTable *ft, World *w )
{
  struct fann_rt *new_ann = (struct fann_rt *) RTAlloc( w, sizeof( struct fann_rt ) );
  if( !new_ann ) return NULL;
  new_ann->first_layer = NULL;
  new_ann->weights = NULL;
  new_ann->connections = NULL;
  new_ann->output = NULL;
  return new_ann;
}

void fann_rt_destroy( struct fann_rt *ann, InterfaceTable *ft, World *w )
{
  if( !ann ) return;
  RTFree( w, ann->weights );
  RTFree( w, ann->connections );
  if( ann->first_layer ) RTFree( w, ann->first_layer->first_neuron );
  RTFree( w, ann->first_layer );
  RTFree( w, ann->output );
  RTFree( w, ann );
}

FANN_EXTERNAL fann_type *FANN_API fann_rt_run(struct fann_rt * ann, fann_type * input)
{
    struct fann_neuron *neuron_it, *last_neuron, *neurons, **neuron_pointers;
    unsigned int i, num_connections, num_input, num_output;
    fann_type neuron_sum, *output;
    fann_type *weights;
    struct fann_layer *layer_it, *last_layer;
    unsigned int activation_function;
    fann_type steepness;

    /* store some variabels local for fast access */
    struct fann_neuron *first_neuron = ann->first_layer->first_neuron;

    fann_type max_sum;

    /* first set the input */
    num_input = ann->num_input;
    for(i = 0; i != num_input; i++)
    {
        first_neuron[i].value = input[i];
    }
    /* Set the bias neuron in the input layer */
    (ann->first_layer->last_neuron - 1)->value = 1;

    last_layer = ann->last_layer;
    for(layer_it = ann->first_layer + 1; layer_it != last_layer; layer_it++)
    {
        last_neuron = layer_it->last_neuron;
        for(neuron_it = layer_it->first_neuron; neuron_it != last_neuron; neuron_it++)
        {
            if(neuron_it->first_con == neuron_it->last_con)
            {
                /* bias neurons */
                neuron_it->value = 1;
                continue;
            }

            activation_function = neuron_it->activation_function;
            steepness = neuron_it->activation_steepness;

            neuron_sum = 0;
            num_connections = neuron_it->last_con - neuron_it->first_con;
            weights = ann->weights + neuron_it->first_con;

            if(ann->connection_rate >= 1)
            {
                if(ann->network_type == FANN_NETTYPE_SHORTCUT)
                {
                    neurons = ann->first_layer->first_neuron;
                }
                else
                {
                    neurons = (layer_it - 1)->first_neuron;
                }


                /* unrolled loop start */
                i = num_connections & 3;    /* same as modulo 4 */
                switch (i)
                {
                    case 3:
                        neuron_sum += fann_mult(weights[2], neurons[2].value);
                    case 2:
                        neuron_sum += fann_mult(weights[1], neurons[1].value);
                    case 1:
                        neuron_sum += fann_mult(weights[0], neurons[0].value);
                    case 0:
                        break;
                }

                for(; i != num_connections; i += 4)
                {
                    neuron_sum +=
                        fann_mult(weights[i], neurons[i].value) +
                        fann_mult(weights[i + 1], neurons[i + 1].value) +
                        fann_mult(weights[i + 2], neurons[i + 2].value) +
                        fann_mult(weights[i + 3], neurons[i + 3].value);
                }
                /* unrolled loop end */

                /*
                 * for(i = 0;i != num_connections; i++){
                 * printf("%f += %f*%f, ", neuron_sum, weights[i], neurons[i].value);
                 * neuron_sum += fann_mult(weights[i], neurons[i].value);
                 * }
                 */
            }
            else
            {
                neuron_pointers = ann->connections + neuron_it->first_con;

                i = num_connections & 3;    /* same as modulo 4 */
                switch (i)
                {
                    case 3:
                        neuron_sum += fann_mult(weights[2], neuron_pointers[2]->value);
                    case 2:
                        neuron_sum += fann_mult(weights[1], neuron_pointers[1]->value);
                    case 1:
                        neuron_sum += fann_mult(weights[0], neuron_pointers[0]->value);
                    case 0:
                        break;
                }

                for(; i != num_connections; i += 4)
                {
                    neuron_sum +=
                        fann_mult(weights[i], neuron_pointers[i]->value) +
                        fann_mult(weights[i + 1], neuron_pointers[i + 1]->value) +
                        fann_mult(weights[i + 2], neuron_pointers[i + 2]->value) +
                        fann_mult(weights[i + 3], neuron_pointers[i + 3]->value);
                }
            }

            neuron_sum = fann_mult(steepness, neuron_sum);

            max_sum = 150/steepness;
            if(neuron_sum > max_sum)
                neuron_sum = max_sum;
            else if(neuron_sum < -max_sum)
                neuron_sum = -max_sum;

            neuron_it->sum = neuron_sum;

            fann_activation_switch(activation_function, neuron_sum, neuron_it->value);
        }
    }

    /* set the output */
    output = ann->output;
    num_output = ann->num_output;
    neurons = (ann->last_layer - 1)->first_neuron;
    for(i = 0; i != num_output; i++)
    {
        output[i] = neurons[i].value;
    }
    return ann->output;
}

struct fann_train_data * fann_rt_create_train (
    unsigned int num_data, unsigned int num_input, unsigned int num_output,
    InterfaceTable *ft, World *w )
{
    unsigned int i;
    fann_type *data_input, *data_output;

    struct fann_train_data *data = (struct fann_train_data *)
        RTAlloc( w, sizeof( struct fann_train_data ) );

    data->input = 0;
    data->output = 0;

    if(data == NULL){
        Print("Ann: Could not allocate training data\n");
        return NULL;
    }

    fann_init_error_data((struct fann_error *) data);

    data->num_data = num_data;
    data->num_input = num_input;
    data->num_output = num_output;

    data->input = (fann_type **) RTAlloc(w, num_data * sizeof(fann_type *));
    if(data->input == NULL)
    {
        Print("Ann: Could not allocate training data\n");
        fann_rt_destroy_train(data, ft, w);
        return NULL;
    }
    data->input[0] = NULL; // prevent faulty free

    data->output = (fann_type **) RTAlloc(w, num_data * sizeof(fann_type *));
    if(data->output == NULL)
    {
        Print("Ann: Could not allocate training data\n");
        fann_rt_destroy_train(data, ft, w);
        return NULL;
    }
    data->output[0] = NULL; // prevent faulty free

    data_input = (fann_type *) RTAlloc(w, num_input * num_data * sizeof(fann_type));
    if(data_input == NULL)
    {
        Print("Ann: Could not allocate training data\n");
        fann_rt_destroy_train(data, ft, w);
        return NULL;
    }

    data_output = (fann_type *) RTAlloc(w, num_output * num_data * sizeof(fann_type));
    if(data_output == NULL)
    {
        Print("Ann: Could not allocate training data\n");
        fann_rt_destroy_train(data, ft, w);
        return NULL;
    }

    for( i = 0; i != num_data; i++)
    {
        data->input[i] = data_input;
        data_input += num_input;

        data->output[i] = data_output;
        data_output += num_output;
    }

    return data;
}

void fann_rt_destroy_train( struct fann_train_data *data, InterfaceTable *ft, World *w )
{
    if( data->input )
        RTFree( w, data->input[0] );
    if( data->output )
        RTFree( w, data->output[0] );
    RTFree( w, data->input );
    RTFree( w, data->output );
}

#endif // !FIXEDFANN
