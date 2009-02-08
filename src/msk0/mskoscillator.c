#include <glib.h>
#include <math.h>

#include "msk0.h"
#include "mskinternal.h"


typedef struct _MskOscillatorState
{
    float *table;
    float phase;
    
} MskOscillatorState;


static inline float sine_function(float x)
{
    // TODO: The man page says this isn't ISO C..
    // if it's indeed a problem, it'll be changed to sin(), and appropiate casting.
    return sinf(x * (2 * G_PI));
}


void msk_oscillator_activate(MskModule *self, void *state)
{
    MskOscillatorState *ostate = state;
    int i;
    
    ostate->phase = 0;
    ostate->table = g_new0(float, 1024);
    
    for ( i = 0; i < 1024; i++ )
    {
        ostate->table[i] = sine_function((float)i / 1024.0f);
    }
}


void msk_oscillator_deactivate(MskModule *self, void *state)
{
    MskOscillatorState *ostate = state;
    
    g_free(ostate->table);
}


void msk_oscillator_process(MskModule *self, int start, int frames, void *state)
{
    float *output = (float *)msk_module_get_output_buffer(self, "output") + start;
    const float *phase_in = (const float *)msk_module_get_input_buffer(self, "phase") + start;
    const float *frequency = (const float *)msk_module_get_input_buffer(self, "frequency") + start;
    
    MskOscillatorState *ostate = state;
    
    while ( frames-- )
    {
        float phase = (ostate->phase + *phase_in++ * 1024);
        int iphase = (int)(phase);
        int index1 = iphase & 1023;
        int index2 = (iphase + 1) & 1023;
        float frac = phase - (float)(int)phase;
        
        *output++ = ostate->table[index1] +
            frac * (ostate->table[index2] - ostate->table[index1]);
        
        ostate->phase += *frequency++ / (44100.0f / 1024.0f);
        
        ostate->phase -= (float)((int)(ostate->phase / 1024) * 1024);
    }
}



MskModule *msk_oscillator_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "oscillator", msk_oscillator_process);
    
    msk_add_state(mod, msk_oscillator_activate, msk_oscillator_deactivate,
                  sizeof(MskOscillatorState));
    
    msk_add_input_port(mod, "frequency", MSK_AUDIO_DATA, 440.0f);
    msk_add_input_port(mod, "phase", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    
    msk_add_integer_property(mod, "wave_type", 1);
    
    return mod;
}


void msk_pitchtofrequency_process(MskModule *self, int start, int frames, void *state)
{
    const float * const pitch = msk_module_get_input_buffer(self, "pitch");
    float *freq = msk_module_get_output_buffer(self, "frequency");
    int i;
    
    // This cache is just a temporary hack until I get message ports.
    float old_in = -1, cached_out = 0;
    
    for ( i = start; i < start + frames; i++ )
    {
        if ( pitch[i] != old_in )
        {
            old_in = pitch[i];
            cached_out = powf(2, (pitch[i]-69.0f)/12.0f)*440.0f;
        }
        
        freq[i] = cached_out;
    }
}

MskModule *msk_pitchtofrequency_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "pitch to frequency",
                            msk_pitchtofrequency_process);
    
    msk_add_input_port(mod, "pitch", MSK_AUDIO_DATA, 69.0f);
    msk_add_output_port(mod, "frequency", MSK_AUDIO_DATA);
    
    return mod;
}
