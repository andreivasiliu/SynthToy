#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"



/**********************************************************************
 * Module: AddMul
 * Ports: input -> output
 * Properties: add, mul
 **********************************************************************/

static void msk_addmul_process(MskModule *self, int start, int frames, void *state)
{
    const float *in = msk_module_get_input_buffer(self, "input");
    float *out      = msk_module_get_output_buffer(self, "output");
    const float add = *(float*)msk_module_get_property_buffer(self, "add");
    const float mul = *(float*)msk_module_get_property_buffer(self, "mul");
    int i;

    for ( i = start; i < start + frames; i++ )
        out[i] = (in[i] + add) * mul;
}

MskModule *msk_addmul_create(MskContainer *parent)
{
    MskModule *mod;

    mod = msk_module_create(parent, "addmul", msk_addmul_process);

    msk_add_input_port(mod, "in", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);
    msk_add_float_property(mod, "add", 0.0f);
    msk_add_float_property(mod, "mul", 1.0f);

    return mod;
}



/**********************************************************************
 * Module: Add
 * Ports: in#, ... -> out
 * Properties: n/a
 **********************************************************************/

static void msk_add_process(MskModule *self, int start, int frames, void *state)
{
    float *out = msk_module_get_output_buffer(self, "out");
    float **inputs = (float **) self->buffer_groups.group[0];
    int ins = self->buffer_groups.group_size[0];
    int i, j;

    for ( i = start; i < start + frames; i++ )
    {
        out[i] = inputs[0][i] + inputs[1][i];

        for ( j = 2; j < ins; j++ )
            out[i] += inputs[j][i];
    }
}

static void msk_add_addport(MskModule *self)
{
    MskPort *port = msk_add_input_port(self, "in#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);
}

static void msk_add_removeport(MskModule *self)
{
    msk_remove_input_port(self);
}


MskModule *msk_add_create(MskContainer *parent)
{
    MskModule *mod;
    MskPort *port;

    mod = msk_module_create(parent, "add", msk_add_process);

    port = msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);
    port = msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 0.0f);
    msk_add_port_to_buffer_group(port, 0);

    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    msk_dynamic_ports(mod, 1, msk_add_addport, msk_add_removeport);

    return mod;
}



/**********************************************************************
 * Module: Mul
 * Ports: in#, ... -> out
 * Properties: n/a
 **********************************************************************/

static void msk_mul_process(MskModule *self, int start, int frames, void *state)
{
    float *out = msk_module_get_output_buffer(self, "out");
    float **inputs = (float **) self->buffer_groups.group[0];
    int ins = self->buffer_groups.group_size[0];
    int i, j;

    for ( i = start; i < start + frames; i++ )
    {
        out[i] = inputs[0][i] * inputs[1][i];

        for ( j = 2; j < ins; j++ )
            out[i] *= inputs[j][i];
    }
}

static void msk_mul_addport(MskModule *self)
{
    MskPort *port = msk_add_input_port(self, "in#", MSK_AUDIO_DATA, 1.0f);
    msk_add_port_to_buffer_group(port, 0);
}

static void msk_mul_removeport(MskModule *self)
{
    msk_remove_input_port(self);
}


MskModule *msk_mul_create(MskContainer *parent)
{
    MskModule *mod;
    MskPort *port;

    mod = msk_module_create(parent, "mul", msk_mul_process);

    port = msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 1.0f);
    msk_add_port_to_buffer_group(port, 0);
    port = msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 1.0f);
    msk_add_port_to_buffer_group(port, 0);

    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    msk_dynamic_ports(mod, 1, msk_mul_addport, msk_mul_removeport);

    return mod;
}

