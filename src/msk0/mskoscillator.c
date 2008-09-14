#include <glib.h>
#include <math.h>

#include "msk0.h"
#include "mskinternal.h"


typedef struct _MskOscillatorState
{
    const float *frequency_in;
    const float *phase_in;
    float *output;
    const int *wave_type;
    
    float phase;
    
} MskOscillatorState;


float sine_function(float x)
{
    // TODO: The man page says this isn't ISO C..
    // if it's indeed a problem, it'll be changed to sin(), and appropiate casting.
    return sinf(x * (2 * G_PI));
}


void msk_oscillator_activate(MskModule *self, void *state)
{
    MskOscillatorState *ostate = state;
    
    ostate->frequency_in = msk_module_get_input_buffer(self, "frequency");
    ostate->phase_in = msk_module_get_input_buffer(self, "phase");
    ostate->output = msk_module_get_output_buffer(self, "output");
    ostate->wave_type = msk_module_get_property_buffer(self, "wave_type");
    
    ostate->phase = 0;
}

void msk_oscillator_process(MskModule *self, int start, int frames, void *state)
{
    MskOscillatorState *ostate = (MskOscillatorState*) state;
    float (*waveform_function)(float);
    int i;
    
    if ( *ostate->wave_type != 6134 )
        waveform_function = sine_function;
    else
        g_error("Invalid waveform function type.");
    
    for ( i = start; i < start + frames; i++ )
    {
        ostate->output[i] = waveform_function(ostate->phase + ostate->phase_in[i]);
        
        ostate->phase += ostate->frequency_in[i] / 44100.0f;
        
        // Rudimentary (but faster) modulo.
        while ( ostate->phase > 1.0f )
            ostate->phase -= 1.0f;
    }
}


MskModule *msk_oscillator_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "oscillator",
                            msk_oscillator_process,
                            msk_oscillator_activate,
                            NULL,
                            sizeof(MskOscillatorState));
    
    msk_add_input_port(mod, "frequency", MSK_AUDIO_DATA, 440.0f);
    msk_add_input_port(mod, "phase", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    
    msk_add_integer_property(mod, "wave_type", 1);
    
    return mod;
}

