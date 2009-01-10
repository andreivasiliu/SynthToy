#include <glib.h>
#include <glib/gprintf.h>

#include "msk0.h"


void whatiwant()
{
    MskContainer *world, *instr;
    MskModule *out, *iout, *osc, *pitch, *vel, *mul1, *p2f;
    GTimer *timer;
    
    float *buffer_out;
    int i;
    
    world = msk_world_create(44100, 512);
    out = msk_output_create_with_name(world, "audio1", MSK_AUDIO_DATA);
    
    instr = msk_instrument_create(world);
    iout = msk_output_create_with_name(instr, "out1", MSK_AUDIO_DATA);
    
    osc = msk_oscillator_create(instr);
    pitch = msk_voicepitch_create(instr);
    vel = msk_voicevelocity_create(instr);
    mul1 = msk_mul_create(instr);
    p2f = msk_pitchtofrequency_create(instr);
    
    msk_connect_ports(pitch, "pitch", p2f, "pitch");
    msk_connect_ports(p2f, "frequency", osc, "frequency");
    msk_connect_ports(osc, "output", mul1, "in1");
    msk_connect_ports(vel, "velocity", mul1, "in2");
    msk_connect_ports(mul1, "out", iout, "out1");
    
    msk_connect_ports(instr->module, "out1", out, "audio1");
    
    buffer_out = msk_module_get_output_buffer(world->module, "audio1");
    
    msk_world_prepare(instr);
    
    msk_message_note_on(world->module->world, 40, 127);
    msk_message_note_on(world->module->world, 60, 127);
    msk_message_note_on(world->module->world, 70, 127);
    
    timer = g_timer_new();
    
    for ( i = 0; i < 20; i++ )
        msk_world_run(instr);
    
    g_timer_start(timer);
    for ( i = 0; i < 1000; i++ )
        msk_world_run(instr);
    g_timer_stop(timer);
    
    g_print("Time: %f.\n", g_timer_elapsed(timer, NULL));
}

int main()
{
    whatiwant();
    
    return 0;
}

