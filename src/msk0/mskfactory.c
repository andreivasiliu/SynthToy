#include <glib.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


const struct
{
    const char *name;
    MskModule *(*create_function)(MskContainer *parent);
} create_functions[] =
{
    { "oscillator", &msk_oscillator_create },
    { "input", &msk_input_create },
    { "output", &msk_output_create },
    { "voicenumber", &msk_voicenumber_create },
    { "constant", &msk_constant_create },
    { "autoconstant", &msk_autoconstant_create },
    { "pitch to frequency", &msk_pitchtofrequency_create },
    { "addmul", &msk_addmul_create },
    { "add", &msk_add_create },
    { "mul", &msk_mul_create },
    { "voiceactive", &msk_voiceactive_create },
    { "voicepitch", &msk_voicepitch_create },
    { "voicevelocity", &msk_voicevelocity_create },
    { "ADSR", &msk_adsr_create },
    { NULL, NULL }
};


MskModule *msk_factory_create_module(const char *name, MskContainer *parent)
{
    int i;

    for ( i = 0; create_functions[i].name; i++ )
    {
        if ( !strcmp(create_functions[i].name, name) )
            return create_functions[i].create_function(parent);
    }

    return NULL;
}

