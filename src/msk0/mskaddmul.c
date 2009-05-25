#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"



/**********************************************************************
 * Module: AddMul
 * Ports: input -> output
 * Properties: add, mul
 **********************************************************************/

void msk_addmul_process(MskModule *self, int start, int frames, void *state)
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

void msk_add_process(MskModule *self, int start, int frames, void *state)
{
    const float * const in1 = msk_module_get_input_buffer(self, "in1");
    const float * const in2 = msk_module_get_input_buffer(self, "in2");
    float *out = msk_module_get_output_buffer(self, "out");
    int i;

    for ( i = start; i < start + frames; i++ )
        out[i] = in1[i] + in2[i];
}

MskModule *msk_add_create(MskContainer *parent)
{
    MskModule *mod;

    mod = msk_module_create(parent, "add", msk_add_process);

    msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 0.0f);
    msk_add_input_port(mod, "in#", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    return mod;
}



/**********************************************************************
 * Module: Mul
 * Ports: in#, ... -> out
 * Properties: n/a
 **********************************************************************/

void msk_mul_process(MskModule *self, int start, int frames, void *state)
{
    const float * const in1 = msk_module_get_input_buffer(self, "in1");
    const float * const in2 = msk_module_get_input_buffer(self, "in2");
    float *out = msk_module_get_output_buffer(self, "out");
    int i;

    for ( i = start; i < start + frames; i++ )
        out[i] = in1[i] * in2[i];
}

MskModule *msk_mul_create(MskContainer *parent)
{
    MskModule *mod;

    mod = msk_module_create(parent, "mul", msk_mul_process);

    msk_add_input_port(mod, "in1", MSK_AUDIO_DATA, 1.0f);
    msk_add_input_port(mod, "in2", MSK_AUDIO_DATA, 1.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);

    return mod;
}

