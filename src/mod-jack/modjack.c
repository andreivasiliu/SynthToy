#include <jack/jack.h>
#include <jack/midiport.h>

#include "modjack.h"

// TODO:
//  - not all failure points are checked
//  - sample rate callback
//  - remove this C++ style comment. :P


typedef struct _ModJackState
{
    ProcessCallback process_func;
    EventCallback event_func;
    void *instance_data;
    
    jack_client_t *client;
    jack_port_t *in_port;
    jack_port_t *out_port;
    jack_port_t *midi_in;
    jack_nframes_t sample_rate;
} ModJackState;


int modjack_process(jack_nframes_t nframes, void *arg)
{
    ModJackState *state = (ModJackState*) arg;
    jack_default_audio_sample_t *in = jack_port_get_buffer(state->in_port, nframes);
    jack_default_audio_sample_t *out = jack_port_get_buffer(state->out_port, nframes);
    void *midi_in = jack_port_get_buffer(state->midi_in, nframes);
    
    jack_midi_event_t event;
    
    while ( jack_midi_event_get(&event, midi_in, 0) == 0 )
    {
        state->event_func(event.time, 0, event.buffer, event.size, NULL);
    }
    
    state->process_func((float*) in, (float*) out, (int) nframes,
                        (int) state->sample_rate, state->instance_data);
    
    return 0;
}


int modjack_samplerate_change(jack_nframes_t nframes, void *arg)
{
    ModJackState *state = (ModJackState*) arg;
    
    state->sample_rate = nframes;
    return 0;
}


void *modjack_init(ProcessCallback process_func, EventCallback event_func,
                   void *arg, char **errmsg)
{
    jack_client_t *client;
    ModJackState *state;
    
    client = jack_client_open("modjack", 0, NULL);
    
    if ( !client )
    {
        *errmsg = "Jack server not found..";
        return NULL;
    }
    
    state = calloc(1, sizeof(ModJackState));
    state->client = client;
    state->process_func = process_func;
    state->event_func = event_func;
    state->instance_data = arg;
    
    state->sample_rate = jack_get_sample_rate(client);
    
    state->in_port  = jack_port_register(client, "in",  JACK_DEFAULT_AUDIO_TYPE,
                                         JackPortIsInput, 0);
    state->out_port = jack_port_register(client, "out", JACK_DEFAULT_AUDIO_TYPE,
                                         JackPortIsOutput, 0);
    state->midi_in  = jack_port_register(client, "min", JACK_DEFAULT_MIDI_TYPE,
                                         JackPortIsInput, 0);
    
    jack_set_process_callback(state->client, modjack_process, (void*) state);
    
    *errmsg = NULL;
    
    return state;
}


void modjack_fini(void *state)
{
    ModJackState *mjstate = (ModJackState*) state;
    
    jack_client_close(mjstate->client);
    free(mjstate);
}


void modjack_activate(void *state)
{
    ModJackState *mjstate = (ModJackState*) state;
    
    jack_activate(mjstate->client);
    
    jack_connect(mjstate->client,
                 jack_port_name(mjstate->out_port),
                 "system:playback_1");
    
    jack_connect(mjstate->client,
                 jack_port_name(mjstate->out_port),
                 "system:playback_2");
}


void modjack_deactivate(void *state)
{
    jack_deactivate(((ModJackState*)state)->client);
}
