#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <cairo.h>
#include <string.h>

#include "msk0/msk0.h"
#include "gmsk/gmsk.h"
#include "header.h"


/* Note: This file was created purely for testing. It will go away. */

MskContainer *aural_root;

extern void virkb_noteon(int note);
extern void virkb_noteoff(int note);
extern void draw_module(MskModule *mod, long x, long y);

extern void on_editor_invalidated(void *userdata);
extern void on_module_selected(MskModule *module, void *userdata);

float array[512];
float array2[512];

gdouble processing_time;

struct voice_type
{
    MskModule *osc, *lfo, *osc_freq, *lfo_freq, *amp, *freq_add;
} voice;

float note_frequencies[128];
int last_note = -1;

GTimer *timer;

void aural_init()
{
    MskModule *output1, *output2;

    int i;

    note_frequencies[69] = 440;
    for ( i = 70; i < 128; i++ )
        note_frequencies[i] = note_frequencies[i-1] * 1.0594630943f;
    for ( i = 68; i >= 0; i-- )
        note_frequencies[i] = note_frequencies[i+1] / 1.0594630943f;

    aural_root = msk_world_create(44100, 256);
    output1 = msk_output_create_with_name(aural_root, "audio1", MSK_AUDIO_DATA);
    output2 = msk_output_create_with_name(aural_root, "audio2", MSK_AUDIO_DATA);

    msk_world_prepare(aural_root);

    gmsk_init(aural_root);
    gmsk_set_invalidate_callback(&on_editor_invalidated, NULL);
    gmsk_set_select_module_callback(&on_module_selected, NULL);

    gmsk_draw_module_at(output1, 540, 70);
    gmsk_draw_module_at(output2, 540, 110);

    timer = g_timer_new();
}



void process_func(float *in, float *out_left, float *out_right, int nframes,
        int sample_rate, void *data)
{
    float *buffer;
    int i, j;

    gmsk_lock_mutex();

    for ( i = 0; i < nframes; i += 256 )
    {
        g_timer_start(timer);
        msk_world_run(aural_root);
        g_timer_stop(timer);

        processing_time += g_timer_elapsed(timer, NULL);

        buffer = msk_module_get_output_buffer(aural_root->module, "audio2");
        memcpy(out_right + i, buffer, 256 * sizeof(float));
        for ( j = i; j < i + 256; j++ )
            out_right[j] = 0.2 * out_right[j];

        buffer = msk_module_get_output_buffer(aural_root->module, "audio1");
        memcpy(out_left + i, buffer, 256 * sizeof(float));
        for ( j = i; j < i + 256; j++ )
            out_left[j] = 0.2 * out_left[j];

        if ( i < 512 )
            memcpy(array + i, buffer, 256 * sizeof(float));
    }

    gmsk_unlock_mutex();
}

void uint8_to_binary(int from, char *to)
{
    int i;

    for ( i = 0; i < 8; i++ )
        to[7-i] = (from & (1 << i)) ? '1' : '0';

    to[i] = 0;
}

#define MIDI_CHANNEL(byte)         ((byte & 0xF) + 1)
#define MIDI_TYPE(byte)            ((byte >> 4) & 0xF)
#define MIDI_14BIT(lbits, mbits)   (((int)mbits << 7) | ((int)lbits))


void event_func(int nframes, int type, void *event_data, int event_size, void *data)
{
    struct voice_type *v;
    unsigned char sb[9], db1[9], db2[9];
    unsigned char *message;

    if ( type != 1 )
        return;

    // TODO: Isn't a gmsk_lock_mutex needed here?

    message = (unsigned char*) event_data;

    v = &voice;

    switch ( MIDI_TYPE(message[0]) )
    {
    case 0x8:  /* 1000 */
        g_print("Ch%d: NoteOff, key %d, velocity %d.\n",
                MIDI_CHANNEL(message[0]), message[1], message[2]);

        virkb_noteoff(message[1]);
        gmsk_lock_mutex();
        msk_message_note_off(aural_root->module->world, MIDI_CHANNEL(message[0]),
                message[1], message[2]);
        gmsk_unlock_mutex();

        break;
    case 0x9:  /* 1001 */
        g_print("Ch%d: NoteOn, key %d, velocity %d.\n",
                MIDI_CHANNEL(message[0]), message[1], message[2]);

        if ( message[2] )
            virkb_noteon(message[1]);
        else
            virkb_noteoff(message[1]);

        gmsk_lock_mutex();
        msk_message_note_on(aural_root->module->world, MIDI_CHANNEL(message[0]),
                message[1], message[2]);
        gmsk_unlock_mutex();

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
    case 0xC:  /* 1100 */
        g_print("Ch%d: ProgramChange, program %d.\n",
                MIDI_CHANNEL(message[0]), message[1]);

        break;
    case 0xD:
        g_print("Ch%d: ChannelPressure, value %d.\n",
                MIDI_CHANNEL(message[0]), message[1]);

        gmsk_lock_mutex();
        msk_message_channel_pressure(aural_root->module->world,
                MIDI_CHANNEL(message[0]), message[1]);
        gmsk_unlock_mutex();

        break;
    case 0xE:
        g_print("Ch%d: PitchWheelChange, value %d.\n",
                MIDI_CHANNEL(message[0]), MIDI_14BIT(message[1], message[2]));

        gmsk_lock_mutex();
        msk_message_pitch_bend(aural_root->module->world, MIDI_CHANNEL(message[0]),
                MIDI_14BIT(message[1], message[2]));
        gmsk_unlock_mutex();

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

