#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/**********************************************************************
 * Module: FIR Filter
 * Ports: a#, ... -> out
 * Properties: n/a
 **********************************************************************/

typedef struct _MskFIRFilterState
{

} MskFIRFilterState;


static void msk_firfilter_activate(MskModule *self, void *state)
{

}

static void msk_firfilter_deactivate(MskModule *self, void *state)
{

}

static void msk_firfilter_process(MskModule *self, int start, int frames, void *state)
{
    const float *in = msk_module_get_output_buffer(self, "in");
    float *out = msk_module_get_output_buffer(self, "out");
    float **a = (float **) self->buffer_groups.group[0];
    int i, g;

    for ( i = start; i < start + frames; i++ )
    {
        out[i] = a[0][i] * in[i];
        for ( g = 0; g < self->buffer_groups.group_size[0]; g++ )
            ;
    }

}

static void msk_firfilter_addport(MskModule *self)
{
    MskPort *port = msk_add_input_port(self, "a#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);

    // update state
}

static void msk_firfilter_removeport(MskModule *self)
{
    msk_remove_input_port(self);

    // update state
}


MskModule *msk_firfilter_create(MskContainer *parent)
{
    MskModule *mod;
    MskPort *port;

    mod = msk_module_create(parent, "FIR filter", msk_firfilter_process);
    msk_add_state(mod, msk_firfilter_activate, msk_firfilter_deactivate,
            sizeof(MskFIRFilterState));

    msk_add_input_port(mod, "in", MSK_AUDIO_DATA, 0.0f);
    port = msk_add_input_port(mod, "a0", MSK_AUDIO_DATA, 1.0f);
    msk_add_port_to_buffer_group(port, 0);
    port = msk_add_input_port(mod, "a#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    msk_dynamic_ports(mod, 1, msk_firfilter_addport, msk_firfilter_removeport);

    return mod;
}



/**********************************************************************
 * Module: IIR Filter
 * Ports: a#, ..., b#, ... -> out
 * Properties: n/a
 **********************************************************************/

typedef struct _MskIIRFilterState
{

} MskIIRFilterState;


static void msk_iirfilter_activate(MskModule *self, void *state)
{

}

static void msk_iirfilter_deactivate(MskModule *self, void *state)
{

}

static void msk_iirfilter_process(MskModule *self, int start, int frames, void *state)
{


}

static void msk_iirfilter_addport(MskModule *self)
{
    MskPort *port;

    port = msk_add_input_port(self, "a#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);
    port = msk_add_input_port(self, "b#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);

    // update state
}

static void msk_iirfilter_removeport(MskModule *self)
{
    msk_remove_input_port(self);
    msk_remove_input_port(self);

    // update state
}


MskModule *msk_iirfilter_create(MskContainer *parent)
{
    MskModule *mod;
    MskPort *port;

    mod = msk_module_create(parent, "IIR filter", msk_iirfilter_process);
    msk_add_state(mod, msk_iirfilter_activate, msk_iirfilter_deactivate,
            sizeof(MskIIRFilterState));

    msk_add_input_port(mod, "in", MSK_AUDIO_DATA, 0.0f);
    port = msk_add_input_port(mod, "b0", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 1);
    port = msk_add_input_port(mod, "a#", MSK_AUDIO_DATA, 1.0f);
    msk_add_port_to_buffer_group(port, 0);
    port = msk_add_input_port(mod, "b#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 1);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    msk_dynamic_ports(mod, 2, msk_iirfilter_addport, msk_iirfilter_removeport);

    return mod;
}


