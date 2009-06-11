#include <glib.h>
#include <math.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


/**********************************************************************
 * Module: Distort
 * Ports: in -> out
 * Properties: function, preamp
 **********************************************************************/


static void msk_distort_process(MskModule *self, int start, int frames, void *state)
{
    float *out= (float *)msk_module_get_output_buffer(self, "out") + start;
    const float *in = (const float *)msk_module_get_input_buffer(self, "in") + start;
    const float preamp = *(const float *)msk_module_get_property_buffer(self, "preamp");
    const char *function = (const char *)msk_module_get_property_buffer(self, "function");

    if ( !strcmp(function, "softclip") )
    {
        while ( frames-- )
        {
            *(out++) = atan(*(in++) * preamp);
        }
    }
    else if ( !strcmp(function, "hardclip" ) )
    {
        while ( frames-- )
        {
            float amped_in = *(in++) * preamp;

            if ( amped_in < -1 )
                *(out++) = -1;
            else if ( amped_in > 1 )
                *(out++) = 1;
            else
                *(out++) = amped_in;
        }
    }
    else
    {
        while ( frames-- )
        {
            *(out++) = 0;
        }
    }
}


MskModule *msk_distort_create(MskContainer *parent)
{
    MskModule *mod;

    mod = msk_module_create(parent, "distort", msk_distort_process);
    msk_add_input_port(mod, "in", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);
    msk_add_string_property(mod, "function", "softclip");
    msk_add_float_property(mod, "preamp", 1.0f);

    return mod;
}


