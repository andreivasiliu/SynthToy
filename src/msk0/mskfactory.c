#include <glib.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


const struct
{
    const char *name;
    MskModule *(*create_function)(MskContainer *parent);
    gboolean requires_instrument;
} create_module_functions[] =
{
    /* Name        */
    { "oscillator",             &msk_oscillator_create,         FALSE },
    { "input",                  &msk_input_create,              FALSE },
    { "output",                 &msk_output_create,             FALSE },
    { "voicenumber",            &msk_voicenumber_create,        FALSE },
    { "constant",               &msk_constant_create,           FALSE },
    { "pitch to frequency",     &msk_pitchtofrequency_create,   FALSE },
    { "addmul",                 &msk_addmul_create,             FALSE },
    { "add",                    &msk_add_create,                FALSE },
    { "mul",                    &msk_mul_create,                FALSE },
    { "voiceactive",            &msk_voiceactive_create,        TRUE },
    { "voicepitch",             &msk_voicepitch_create,         TRUE },
    { "voicevelocity",          &msk_voicevelocity_create,      TRUE },
    { "pitchbend",              &msk_pitchbend_create,          TRUE },
    { "channelpressure",        &msk_channelpressure_create,    TRUE },
    { "parameter",              &msk_parameter_create,          TRUE },
    { "ADSR",                   &msk_adsr_create,               FALSE },
    { "delay",                  &msk_delay_create,              FALSE },
    { "FIR filter",             &msk_firfilter_create,          FALSE },
    { "IIR filter",             &msk_iirfilter_create,          FALSE },
    { "distort",                &msk_distort_create,            FALSE },

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

static inline MskInstrument *get_instrument(MskContainer *parent)
{
    MskInstrument *instrument = NULL;

    while ( parent && (instrument = parent->instrument) == NULL )
        parent = parent->module->parent;

    return instrument;
}


MskModule *msk_factory_create_module(const char *name, MskContainer *parent, GError **error)
{
    int i;

    for ( i = 0; create_module_functions[i].name; i++ )
    {
        if ( !strcmp(create_module_functions[i].name, name) )
        {
            /* Check if it needs to be inside an instrument. */
            if ( create_module_functions[i].requires_instrument &&
                 !get_instrument(parent) )
            {
                g_set_error(error, 0, 0, "The '%s' module can only be created "
                        "inside an instrument.", name);
                return NULL;
            }

            return create_module_functions[i].create_function(parent);
        }
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
