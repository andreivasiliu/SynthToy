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
    
    ostate->frequency_in = msk_module_get_input_buffer(self, "frequency");
    ostate->phase_in = msk_module_get_input_buffer(self, "phase");
    ostate->output = msk_module_get_output_buffer(self, "output");
    ostate->wave_type = msk_module_get_property_buffer(self, "wave_type");
    
    ostate->phase = 0;
}

void msk_oscillator_process(MskModule *self, int start, int frames, void *state)
{
    MskOscillatorState *ostate = (MskOscillatorState*) state;
    //float (*waveform_function)(float);
    int i;
    
    //if ( *ostate->wave_type != 6134 )
    //    waveform_function = sine_function;
    //else
    //    g_error("Invalid waveform function type.");
    
    for ( i = start; i < start + frames; i++ )
    {
        ostate->output[i] = sine_function(ostate->phase + ostate->phase_in[i]);
        
        ostate->phase += /*ostate->frequency_in[i]*/ 440 / 44100.0f;
        
        // Rudimentary (but faster) modulo.
        if ( ostate->phase > 20000.f )
            while ( ostate->phase > 1.0f )
                ostate->phase -= 1.0f;
    }
}


void msk_oscillator2_activate(MskModule *self, void *state)
{
    MskOscillatorState *ostate = state;
    int i;
    
    ostate->phase = 0;
    ostate->table = g_new0(float, 1000);
    
    for ( i = 0; i < 1000; i++ )
    {
        ostate->table[i] = sine_function((float)i / 1000.0f);
    }
}

void msk_oscillator2_process(MskModule *self, int start, int frames, void *state)
{
    float *output = msk_module_get_output_buffer(self, "output") + start;
    const float *phase_in = msk_module_get_input_buffer(self, "phase") + start;
    const float *frequency = msk_module_get_input_buffer(self, "frequency") + start;
    
    MskOscillatorState *ostate = state;
    
    while ( frames-- )
    {
        float phase = (ostate->phase + *phase_in++);
        int iphase = (int)(phase);
        int index1 = iphase % 1000;
        int index2 = (iphase + 1) % 1000;
        float frac = phase - (float)(int)phase;
        
        *output++ = ostate->table[index1] +
            frac * (ostate->table[index2] - ostate->table[index1]);
        
        ostate->phase += *frequency++ / (44.100f);
        
        if ( ostate->phase >= 1000.0f )
            while ( ostate->phase > 1000.0f )
                ostate->phase -= 1000.0f;
    }
}



MskModule *msk_oscillator_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "oscillator",
                            msk_oscillator2_process,
                            msk_oscillator2_activate,
                            NULL,
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
    
    for ( i = start; i < start + frames; i++ )
        freq[i] = powf(2, (pitch[i]-69.0f)/12.0f)*440.0f;
}

MskModule *msk_pitchtofrequency_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "pitch to frequency",
                            msk_pitchtofrequency_process,
                            NULL,
                            NULL,
                            0);
    
    msk_add_input_port(mod, "pitch", MSK_AUDIO_DATA, 69.0f);
    msk_add_output_port(mod, "frequency", MSK_AUDIO_DATA);
    
    return mod;
}
