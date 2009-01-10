#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>

#include "header.h"
#include "msk0/msk0.h"


/* Note: This file was created purely for testing. It will go away. */

GMutex *lock_for_model;

extern void virkb_noteon(int note);
extern void virkb_noteoff(int note);
extern void draw_module(MskModule *mod, long x, long y);

extern float array[512];
extern float array2[512];

MskContainer *cont;
extern MskContainer *current_container;

gdouble processing_time;

struct voice_type
{
    MskModule *osc, *lfo, *osc_freq, *lfo_freq, *amp, *freq_add;
} voice;

float note_frequencies[128];
int last_note = -1;

GTimer *timer;

void init_channel()
{
    MskModule *output;
    
    //voice.osc = msk_oscillator_create(cont);
    //voice.lfo = msk_oscillator_create(cont);
    //voice.osc_freq = msk_constant_create(cont);
    //voice.lfo_freq = msk_constant_create(cont);
    //voice.amp = msk_addmul_create(cont);
    output = msk_output_create_with_name(cont, "audio1", MSK_AUDIO_DATA);
    //voice.freq_add = msk_addmul_create(cont);
    
    //draw_module(voice.osc, 285, 90);
    //draw_module(voice.lfo, 140, 150);
    //draw_module(voice.osc_freq, 40, 50);
    //draw_module(voice.lfo_freq, 35, 150);
    //draw_module(voice.amp, 425, 90);
    //draw_module(voice.freq_add, 150, 50);
    draw_module(output, 540, 90);
    
    //msk_connect_ports(voice.osc_freq, "output", voice.freq_add, "input");
    //msk_connect_ports(voice.freq_add, "output", voice.osc, "frequency");
    //msk_connect_ports(voice.lfo_freq, "output", voice.lfo, "frequency");
    //msk_connect_ports(voice.lfo, "output", voice.osc, "phase");
    //msk_connect_ports(voice.osc, "output", voice.amp, "input");
    //msk_connect_ports(voice.amp, "output", output, audio1);
}

void aural_init()
{
    int i;
    
    note_frequencies[69] = 440;
    for ( i = 70; i < 128; i++ )
        note_frequencies[i] = note_frequencies[i-1] * 1.0594630943f;
    for ( i = 68; i >= 0; i-- )
        note_frequencies[i] = note_frequencies[i+1] / 1.0594630943f;
    
    cont = msk_world_create(44100, 256);
    
    current_container = cont;
    
    init_channel();
    
    timer = g_timer_new();
    lock_for_model = g_mutex_new();
    
    msk_world_prepare(cont);
}



void process_func(float *in, float *out, int nframes, int sample_rate, void *data)
{
    float *buffer;
    int i;
    
    for ( i = 0; i < nframes; i += 256 )
    {
        g_timer_start(timer);
        msk_world_run(cont);
        g_timer_stop(timer);
        
        processing_time += g_timer_elapsed(timer, NULL);
        
        buffer = msk_module_get_output_buffer(cont->module, "audio1");
            
        memcpy(out + i, buffer, 256 * sizeof(float));
        
        if ( i < 512 )
            memcpy(array + i, buffer, 256 * sizeof(float));
    }
}

void uint8_to_binary(int from, char *to)
{
    int i;
    
    for ( i = 0; i < 8; i++ )
        to[7-i] = (from & (1 << i)) ? '1' : '0';
    
    to[i] = 0;
}

#define MIDI_CHANNEL(byte)         (byte & 0xF)
#define MIDI_TYPE(byte)            ((byte >> 4) & 0xF)
#define MIDI_14BIT(lbits, mbits)   (((int)mbits << 7) | ((int)lbits))


void event_func(int nframes, int type, void *event_data, int event_size, void *data)
{
    struct voice_type *v;
    unsigned char sb[9], db1[9], db2[9];
    unsigned char *message;
    
    if ( type != 1 )
        return;
    
    message = (unsigned char*) event_data;
    
    v = &voice;
    
    switch ( MIDI_TYPE(message[0]) )
    {
    case 0x8:  /* 1000 */
        g_print("Ch%d: NoteOff, key %d, velocity %d.\n",
                MIDI_CHANNEL(message[0]), message[1], message[2]);
        
        virkb_noteoff(message[1]);
        msk_message_note_off(cont->module->world, message[1], message[2]);
        
        break;
    case 0x9:  /* 1001 */
        g_print("Ch%d: NoteOn, key %d, velocity %d.\n",
                MIDI_CHANNEL(message[0]), message[1], message[2]);
        
        if ( message[2] )
            virkb_noteon(message[1]);
        else
            virkb_noteoff(message[1]);
        
        msk_message_note_on(cont->module->world, message[1], message[2]);
        
        /*
        if ( message[2] == 0 && message[1] != last_note )
            break;
            
        msk_module_set_float_property(v->osc_freq, "value", message[1]);
        msk_module_set_float_property(v->amp, "mul", (float) message[2] / 127);
        last_note = message[1];
        
        if ( message[2] )
            g_print("Frequency: %f.\n", note_frequencies[message[1]]);
        */
        
        break;
    case 0xB:  /* 1011 */
        g_print("Ch%d: ControlChange, control %d, value %d.\n",
                MIDI_CHANNEL(message[0]), message[1], message[2]);
        
        /*
        if ( message[1] == 1 )
            msk_module_set_float_property(v->lfo_freq, "value",
                                          ((float) message[2] / 127) * 10);
        */
        
        break;
    case 0xD:
        g_print("Ch%d: ChannelPressure, value %d.\n",
                MIDI_CHANNEL(message[0]), message[1]);
        /*
        msk_module_set_float_property(v->amp, "mul", 0.5 + (float) message[1] / 127 / 2);
        */
        break;
    case 0xE:
        g_print("Ch%d: PitchWheelChange, value %d.\n",
                MIDI_CHANNEL(message[0]), MIDI_14BIT(message[1], message[2]));
        
        /*
        msk_module_set_float_property(v->freq_add, "add",
                                      (float) MIDI_14BIT(message[1], message[2]) / 8192 - 1);
        */
        
        break;
    default:
        uint8_to_binary(message[0], sb);
        uint8_to_binary(message[1], db1);
        uint8_to_binary(message[2], db2);
        
        g_print("Status: %s, Data1: %s, Data2 byte: %s.\n", sb, db1, db2);
        break;
    }
}

void emulate_note_on(int channel, int key, int velocity)
{
    unsigned char message[3];
    
    message[0] = (0x9 << 4) | (channel & 0xF);
    message[1] = key & 0x7F;
    message[2] = velocity & 0x7F;
    
    event_func(0, 1, message, 3, NULL);
}

void emulate_note_off(int channel, int key)
{
    unsigned char message[3];
    
    message[0] = (0x9 << 4) | (channel & 0xF);
    message[1] = key & 0x7F;
    message[2] = 0;
    
    event_func(0, 1, message, 3, NULL);
}

void emulate_control_change(int channel, int control, int value)
{
    unsigned char message[3];
    
    message[0] = (0xB << 4) | (channel & 0xF);
    message[1] = control & 0x7F;
    message[2] = value & 0x7F;
    
    event_func(0, 1, message, 3, NULL);
}

