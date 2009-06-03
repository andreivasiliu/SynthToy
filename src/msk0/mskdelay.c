#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/**********************************************************************
 * Module: Delay
 * Ports: in -> out
 * Properties: delay
 **********************************************************************/

typedef struct _MskDelayState
{
    float *buffer;
    int current_position;

    /* Split mode. */
    float *write_position;
    float *read_position;
} MskDelayState;


static void msk_delay_activate(MskModule *self, void *state)
{
    int delay = msk_module_get_integer_property(self, "delay");
    MskDelayState *dstate = state;

    dstate->buffer = g_new0(float, delay);
    dstate->current_position = 0;
    dstate->write_position = dstate->buffer;
    dstate->read_position = dstate->buffer;
}


static void msk_delay_deactivate(MskModule *self, void *state)
{
    MskDelayState *dstate = state;

    g_free(dstate->buffer);
}


static void msk_delay_process(MskModule *self, int start, int frames, void *state)
{
    float *out= (float *)msk_module_get_output_buffer(self, "out") + start;
    const float *in = (const float *)msk_module_get_input_buffer(self, "in") + start;
    const int delay = *(const int *)msk_module_get_property_buffer(self, "delay");
    MskDelayState *dstate = state;

    while ( frames-- )
    {
        float *buffer = dstate->buffer + dstate->current_position;
        *(out++) = *buffer;
        *buffer = *(in++);
        dstate->current_position++;

        if ( dstate->current_position >= delay )
            dstate->current_position = 0;
    }
}


static void msk_delay_process_input(MskModule *self, int start, int frames, void *state)
{
    const float *in = (const float *)msk_module_get_input_buffer(self, "in") + start;
    const int delay = *(const int *)msk_module_get_property_buffer(self, "delay");
    MskDelayState *dstate = state;

    while ( frames-- )
    {
        *(dstate->write_position++) = *(in++);
        if ( dstate->write_position - dstate->buffer >= delay )
            dstate->write_position = dstate->buffer;
    }
}


static void msk_delay_process_output(MskModule *self, int start, int frames, void *state)
{
    float *out= (float *)msk_module_get_output_buffer(self, "out") + start;
    const int delay = *(const int *)msk_module_get_property_buffer(self, "delay");
    MskDelayState *dstate = state;

    while ( frames-- )
    {
        *(out++) = *(dstate->read_position++);
        if ( dstate->read_position - dstate->buffer >= delay )
            dstate->read_position = dstate->buffer;
    }
}


static void msk_delay_time_changed(MskProperty *property, void *value)
{
    int *time = value;

    if ( *time < 1 )
        *time = 1024;

    property->owner->delay_limiter = *time;

    msk_container_sort(property->owner->parent);
}


MskModule *msk_delay_create(MskContainer *parent)
{
    MskModule *mod;
    MskProperty *prop;

    mod = msk_module_create(parent, "delay", msk_delay_process);

    msk_add_state(mod, msk_delay_activate, msk_delay_deactivate,
            sizeof(MskDelayState));
    msk_add_split_personality(mod, msk_delay_process_input, msk_delay_process_output);

    msk_add_input_port(mod, "in", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    prop = msk_add_integer_property(mod, "delay", 512); //mod->world->sample_rate);
    msk_property_set_write_callback(prop, &msk_delay_time_changed);
    msk_property_set_flags(prop, MSK_PROPERTY_DEACTIVATES_MODULE);
    msk_property_set_flags(prop, MSK_PROPERTY_DEACTIVATES_MODULE);

    mod->delay_limiter = msk_module_get_integer_property(mod, "delay");

    return mod;
}

