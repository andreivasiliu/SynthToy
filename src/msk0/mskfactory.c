#include <glib.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


const struct
{
    const char *name;
    MskModule *(*create_function)(MskContainer *parent);
} create_module_functions[] =
{
    { "oscillator", &msk_oscillator_create },
    { "input", &msk_input_create },
    { "output", &msk_output_create },
    { "voicenumber", &msk_voicenumber_create },
    { "constant", &msk_constant_create },
    { "pitch to frequency", &msk_pitchtofrequency_create },
    { "addmul", &msk_addmul_create },
    { "add", &msk_add_create },
    { "mul", &msk_mul_create },
    { "voiceactive", &msk_voiceactive_create },
    { "voicepitch", &msk_voicepitch_create },
    { "voicevelocity", &msk_voicevelocity_create },
    { "pitchbend", &msk_pitchbend_create },
    { "channelpressure", &msk_channelpressure_create },
    { "parameter", &msk_parameter_create },
    { "ADSR", &msk_adsr_create },
    { "delay", &msk_delay_create },
    { "FIR filter", &msk_firfilter_create },
    { "IIR filter", &msk_iirfilter_create },
    { "distort", &msk_distort_create },

    { NULL, NULL }
};

const struct
{
    const char *name;
    MskContainer *(*create_function)(MskContainer *parent);
} create_container_functions[] =
{
    { "container", &msk_container_create },
    { "instrument", &msk_instrument_create },
    { NULL, NULL }
};


MskModule *msk_factory_create_module(const char *name, MskContainer *parent)
{
    int i;

    for ( i = 0; create_module_functions[i].name; i++ )
    {
        if ( !strcmp(create_module_functions[i].name, name) )
            return create_module_functions[i].create_function(parent);
    }

    return NULL;
}

MskContainer MSK_API *msk_factory_create_container(const char *name, MskContainer *parent)
{
    int i;

    for ( i = 0; create_container_functions[i].name; i++ )
    {
        if ( !strcmp(create_container_functions[i].name, name) )
            return create_container_functions[i].create_function(parent);
    }

    return NULL;
}
