#include <glib.h>
#include <glib/gprintf.h>

#include "msk0.h"


void whatiwant()
{
    MskContainer *instr, *cont;
    MskModule *out, *osc, *osc2;
    MskModule *m1, *m2, *m3;
    
    float *buffer_out;
    
    instr = msk_world_create(44100, 512);
    
    osc = msk_oscillator_create(instr);
    out = msk_output_create(instr, "out1", MSK_AUDIO_DATA);
    
    cont = msk_container_create(instr);
    m1 = msk_input_create(cont, "cont-input1", MSK_AUDIO_DATA);
    m2 = msk_output_create(cont, "cont-output1", MSK_AUDIO_DATA);
    osc2 = msk_oscillator_create(cont);
    
    msk_connect_ports(m1, "cont-input1", osc2, "frequency");
    msk_connect_ports(osc2, "output", m2, "cont-output1");
    msk_connect_ports(osc, "output", cont->module, "cont-input1");
    
    msk_connect_ports(cont->module, "cont-output1", out, "out1");
    
    msk_world_prepare(instr);
    msk_world_run(instr);
    
    buffer_out = msk_module_get_output_buffer(instr->module, "out1");
}

int main()
{
    whatiwant();
    
    return 0;
}

