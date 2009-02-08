
#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/*
 * Rules and conventions:
 *   - NO globals, NO static variables.
 */

// TODO: Implement quicker access.


void msk_constant_process(MskModule *self, int start, int frames, void *state)
{
    const float value = *(float *)msk_module_get_property_buffer(self, "value");
    float *out = msk_module_get_output_buffer(self, "output");
    int i;
    
    for ( i = start; i < start + frames; i++ )
        out[i] = value;
}


MskModule *msk_constant_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "constant", msk_constant_process);
    
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
    
    mod = msk_module_create(parent, "autoconstant", msk_constant_process);
    
    /* Properties */
    msk_add_float_property(mod, "value", 0.0f);
    
    /* Input/Output ports */
    msk_add_output_port(mod, "output", MSK_AUDIO_DATA);
    
    return mod;
}

