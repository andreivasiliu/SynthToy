#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


typedef struct _MskAddmulState
{
    const float *in;
    float *out;
    
    const float *add;
    const float *mul;
    
} MskAddmulState;


void msk_addmul_activate(MskModule *self, void *state)
{
    MskAddmulState *astate = state;
    
    astate->in  = msk_module_get_input_buffer(self, "input");
    astate->out = msk_module_get_output_buffer(self, "output");
    astate->add = msk_module_get_property_buffer(self, "add");
    astate->mul = msk_module_get_property_buffer(self, "mul");
}

/*
void msk_addmul_process(MskModule *self, int start, int frames, void *state)
{
    MskAddmulState *astate = state;
    int i;
    
    for ( i = start; i < start + frames; i++ )
        astate->out[i] = (astate->in[i] + *astate->add) * *astate->mul;
}*/

void msk_addmul_process(MskModule *self, int start, int frames, void *state)
{
    MskAddmulState *astate = state;
    const float *in = astate->in;
    float *out = astate->out;
    const float add = *astate->add;
    const float mul = *astate->mul;
    int i;
    
    for ( i = start; i < start + frames; i++ )
        out[i] = (in[i] + add) * mul;
}

MskModule *msk_addmul_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "addmul",
                            msk_addmul_process,
                            msk_addmul_activate,
                            NULL,
                            sizeof(MskAddmulState));
    
    msk_add_input_port(mod, "input", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    msk_add_float_property(mod, "add", 0.0f);
    msk_add_float_property(mod, "mul", 1.0f);
    
    return mod;
}

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
    
    mod = msk_module_create(parent, "add",
                            msk_add_process,
                            NULL,
                            NULL,
                            0);
    
    msk_add_input_port(mod, "in1", MSK_AUDIO_DATA, 0.0f);
    msk_add_input_port(mod, "in2", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);
    
    return mod;
}
