#include <glib.h>
#include <math.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


typedef struct _MskOscillatorState
{
    float phase;

} MskOscillatorState;

typedef struct _MskOscillatorGlobalState
{
    float *table;

} MskOscillatorGlobalState;


static inline float sine_function(float x)
{
    // TODO: The man page says this isn't ISO C..
    // if it's indeed a problem, it'll be changed to sin(), and appropiate casting.
    return sinf(x * (2 * G_PI));
}


static void fill_wave_table(char *type, float *table)
{
    int i;

    if ( !strcmp(type, "sine") )
    {
        for ( i = 0; i < 1024; i++ )
            table[i] = sine_function((float)i / 1024.0f);
    }
    else if ( !strcmp(type, "saw") )
    {
        for ( i = 0; i < 1024; i++ )
            table[i] = (float)(i * 2) / 1024 - 1;
    }
    else if ( !strcmp(type, "triangle") )
    {
        for ( i = 0; i < 512; i++ )
            table[i] = (float)(i * 2) / 512 - 1;
        for ( i = 512; i < 1024; i++ )
            table[i] = (2 - (float)((i - 512) * 2) / 512) - 1;
    }
    else if ( !strcmp(type, "square") )
    {
        for ( i = 0; i < 512; i++ )
            table[i] = 1;
        for ( i = 512; i < 1024; i++ )
            table[i] = -1;
    }
    else
        g_error("Unknown wave type.");
}

static void msk_oscillator_activate(MskModule *self, void *state)
{
    MskOscillatorState *ostate = state;

    ostate->phase = 0;
}


static void msk_oscillator_process(MskModule *self, int start, int frames, void *state)
{
    float *output = (float *)msk_module_get_output_buffer(self, "output") + start;
    const float *phase_in = (const float *)msk_module_get_input_buffer(self, "phase") + start;
    const float *frequency = (const float *)msk_module_get_input_buffer(self, "frequency") + start;

    MskOscillatorState *ostate = state;
    MskOscillatorGlobalState *ogstate = self->global_state;

    while ( frames-- )
    {
        float phase = (ostate->phase + *phase_in++ * 1024);
        int iphase = (int)(phase);
        int index1 = iphase & 1023;
        int index2 = (iphase + 1) & 1023;
        float frac = phase - (float)(int)phase;

        *output++ = ogstate->table[index1] +
            frac * (ogstate->table[index2] - ogstate->table[index1]);

        ostate->phase += *frequency++ / (44100.0f / 1024.0f);

        ostate->phase -= (float)((int)(ostate->phase / 1024) * 1024);
    }
}


static void msk_oscillator_type_changed(MskProperty *property, void *value)
{
    char *type = *(char **)value;
    MskOscillatorGlobalState *ogstate = property->owner->global_state;

    if ( !strcmp(type, "saw") || !strcmp(type, "triangle") || !strcmp(type, "square") )
    {
        if ( ogstate )
            fill_wave_table(type, ogstate->table);
    }
    else
    {
        if ( strcmp(type, "sine") )
        {
            g_free(type);
            *(char **)value = g_strdup("sine");
        }

        if ( ogstate )
            fill_wave_table("sine", ogstate->table);
    }
}


static void msk_oscillator_destroy(MskModule *self)
{
    MskOscillatorGlobalState *ogstate = self->global_state;

    g_free(ogstate->table);
}

MskModule *msk_oscillator_create(MskContainer *parent)
{
    MskModule *mod;
    MskProperty *prop;
    MskOscillatorGlobalState *ogstate;

    mod = msk_module_create(parent, "oscillator", msk_oscillator_process);

    msk_add_state(mod, msk_oscillator_activate, NULL,
            sizeof(MskOscillatorState));

    msk_add_input_port(mod, "frequency", MSK_AUDIO_DATA, 440.0f);
    msk_add_input_port(mod, "phase", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);

    prop = msk_add_string_property(mod, "wave_type", "sine");
    msk_property_set_write_callback(prop, &msk_oscillator_type_changed);

    msk_add_destroy_callback(mod, msk_oscillator_destroy);
    ogstate = msk_add_global_state(mod, sizeof(MskOscillatorGlobalState));
    ogstate->table = g_new0(float, 1024);

    fill_wave_table("sine", ogstate->table);

    return mod;
}


static void msk_pitchtofrequency_process(MskModule *self, int start, int frames, void *state)
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
