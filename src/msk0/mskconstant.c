
#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/*
 * Rules and conventions:
 *   - NO globals, NO static variables.
 */

// TODO: Implement quicker access.


typedef struct _MskConstantState
{
    const float *value;
    float *out;
    
} MskConstantState;


void msk_constant_activate(MskModule *self, void *vstate)
{
    MskConstantState *state = vstate;
    
    state->value = msk_module_get_property_buffer(self, "value");
    state->out = msk_module_get_output_buffer(self, "output");
}


// qnote: self, state, frames
void msk_constant_process(MskModule *self, int start, int frames, void *state)
{
    MskConstantState *cstate = state;
    int i;
    
    for ( i = start; i < start + frames; i++ )
        cstate->out[i] = *cstate->value;
}


MskModule *msk_constant_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "constant",
                            msk_constant_process,
                            msk_constant_activate,
                            NULL,
                            sizeof(MskConstantState));
    
    /* Properties */
    msk_add_float_property(mod, "value", 0.0f);
    
    /* Input/Output ports */
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    
    return mod;
}


/* Created automatically for unconnected input ports. */
MskModule *msk_autoconstant_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "autoconstant",
                            msk_constant_process,
                            msk_constant_activate,
                            NULL,
                            sizeof(MskConstantState));
    
    /* Properties */
    msk_add_float_property(mod, "value", 0.0f);
    
    /* Input/Output ports */
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    
    return mod;
}

