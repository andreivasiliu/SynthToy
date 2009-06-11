#include <glib.h>
#include <cairo.h>
#include <string.h>

#include "msk0/msk0.h"
#include "gmsk.h"
#include "gmskinternal.h"


// This should eventually disappear... macros aren't supposed to be
// hard-coded.

MskContainer *macro_simple_sine_generator(MskContainer *parent);

void gmsk_create_macro(char *name)
{
    MskContainer *macro;

    gmsk_lock_mutex();
    msk_container_deactivate(root_container->module->world->root);

    if ( !strcmp(name, "Simple Sine Synth") )
        macro = macro_simple_sine_generator(current_container);
    else
        g_error("Unknown macro: '%s'.", name);

    msk_container_activate(root_container->module->world->root);
    gmsk_unlock_mutex();

    draw_module(macro->module, -1, -1);
    gmsk_invalidate();
}


MskContainer *macro_simple_sine_generator(MskContainer *parent)
{
    MskContainer *macro;
    MskModule *osc, *pitch, *velocity, *active;
    MskModule *p2f, *adsr, *mul1, *mul2, *out;

    /* Create them. */
    macro = msk_instrument_create(parent);

    osc = msk_oscillator_create(macro);
    pitch = msk_voicepitch_create(macro);
    velocity = msk_voicevelocity_create(macro);
    active = msk_voiceactive_create(macro);

    p2f = msk_pitchtofrequency_create(macro);
    adsr = msk_adsr_create(macro);
    mul1 = msk_mul_create(macro);
    mul2 = msk_mul_create(macro);
    out = msk_output_create(macro);

    /* Draw them. */
    draw_module(osc, 245, 65);
    draw_module(pitch, 30, 65);
    draw_module(velocity, 370, 170);
    draw_module(active, 195, 135);
    draw_module(p2f, 110, 65);
    draw_module(adsr, 285, 135);
    draw_module(mul1, 385, 90);
    draw_module(mul2, 470, 115);
    draw_module(out, 555, 115);

    /* Connect them. */
    msk_connect_ports(pitch, "pitch", p2f, "pitch");
    msk_connect_ports(p2f, "frequency", osc, "frequency");

    msk_connect_ports(osc, "output", mul1, "in1");
    msk_connect_ports(active, "gate", adsr, "gate");
    msk_connect_ports(adsr, "out", mul1, "in2");

    msk_connect_ports(mul1, "out", mul2, "in1");
    msk_connect_ports(velocity, "velocity", mul2, "in2");

    msk_connect_ports(mul2, "out", out, "out1");

    return macro;
}


