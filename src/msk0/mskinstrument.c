#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"

void msk_message_note_off(MskWorld *world, short note, short velocity);

/* Find the first free voice, or the oldest used voice. */
static int assign_free_voice(MskInstrument *instrument)
{
    int voices = instrument->container->voices;
    int voice, i;
    
    for ( i = 0; i < voices; i++ )
    {
        voice = (i + instrument->last_voice + 1) % voices;
        if ( !instrument->voice_active[voice] )
        {
            instrument->voice_active[voice] = TRUE;
            instrument->last_voice = voice;
            return voice;
        }
    }
    
    // TODO: Find the oldest one.
     
    /* This instrument has no voices at all. */
    return -1;
}


// TODO: This doesn't handle buggy double NoteOn events.
// The name and MskWorld don't look good.
void msk_message_note_on(MskWorld *world, short note, short velocity)
{
    GList *item;
    
    if ( velocity == 0 )
        return msk_message_note_off(world, note, 0);
    
    // TODO: We're setting all instruments here, but they should be
    // checked for things like channel, key range, etc.
    for ( item = world->instruments; item; item = item->next )
    {
        MskInstrument *instrument = item->data;
        int voice = assign_free_voice(instrument);
        
        /* Instrument with 0 voices? */
        if ( voice < 0 )
            continue;
        
        instrument->voice_note[voice] = note;
        instrument->voice_velocity[voice] = velocity;
    }
}


void msk_message_note_off(MskWorld *world, short note, short velocity)
{
    GList *item;
    int voice;
    
    // TODO: Same comment as on NoteOn. Channel, key range, etc.
    for ( item = world->instruments; item; item = item->next )
    {
        MskInstrument *instrument = item->data;
        int voices = instrument->container->voices;
        
        /* Find the voice with that MIDI note, if any. */
        for ( voice = 0; voice < voices; voice++ )
        {
            // I don't like this instrument->field[voice]..
            // I think instrument[voice]->field might be faster.
            
            if ( instrument->voice_active[voice] &&
                 instrument->voice_note[voice] == note )
            {
                instrument->voice_active[voice] = FALSE;
                //instrument->voice_velocity[voice] = velocity;
            }
        }
    }
}


void msk_voiceactive_process(MskModule *self, int start, int frames, void *state)
{
    float * const out = msk_module_get_output_buffer(self, "gate");
    MskInstrument *instrument = NULL;
    MskContainer *parent = self->parent;
    int i;
    int voice, value;
    
    // TODO: This is WAY TO VERBOSE.
    /* Find the instrument this module resides in. */
    while ( parent && (instrument = parent->instrument) == NULL )
    {
        parent = parent->module->parent;
    }
    
    g_assert(instrument != NULL);
    
    voice = instrument->container->current_voice;
    value = instrument->voice_active[voice];
    
    for ( i = start; i < start + frames; i++ )
        out[i] = value;
}

MskModule *msk_voiceactive_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "voiceactive", msk_voiceactive_process);
    
    msk_add_output_port(mod, "gate", MSK_AUDIO_DATA);
    
    return mod;
}

void msk_voicepitch_process(MskModule *self, int start, int frames, void *state)
{
    float * const out = msk_module_get_output_buffer(self, "pitch");
    MskInstrument *instrument = NULL;
    MskContainer *parent = self->parent;
    int i;
    int voice, value;
    
    // TODO: This is WAY TO VERBOSE.
    /* Find the instrument this module resides in. */
    while ( parent && (instrument = parent->instrument) == NULL )
    {
        parent = parent->module->parent;
    }
    
    g_assert(instrument != NULL);
    
    voice = instrument->container->current_voice;
    value = instrument->voice_note[voice];
    
    for ( i = start; i < start + frames; i++ )
        out[i] = value;
}

MskModule *msk_voicepitch_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "voicepitch", msk_voicepitch_process);
    
    msk_add_output_port(mod, "pitch", MSK_AUDIO_DATA);
    
    return mod;
}

void msk_voicevelocity_process(MskModule *self, int start, int frames, void *state)
{
    float * const out = msk_module_get_output_buffer(self, "velocity");
    MskInstrument *instrument = NULL;
    MskContainer *parent = self->parent;
    int i, voice;
    float value;
    
    // TODO: This is WAY TO VERBOSE.
    /* Find the instrument this module resides in. */
    while ( parent && (instrument = parent->instrument) == NULL )
    {
        parent = parent->module->parent;
    }
    
    g_assert(instrument != NULL);
    
    voice = instrument->container->current_voice;
    value = (float)instrument->voice_velocity[voice] / 127;
    
    for ( i = start; i < start + frames; i++ )
        out[i] = value;
}

MskModule *msk_voicevelocity_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "voicevelocity",
                            msk_voicevelocity_process);
    
    msk_add_output_port(mod, "velocity", MSK_AUDIO_DATA);
    
    return mod;
}


typedef struct _MskADSRState
{
    glong time_passed;
    gboolean gate_on;
    
    float release_value;
} MskADSRState;


void msk_adsr_activate(MskModule *self, void *state)
{
    MskADSRState *adsr = state;
    
    adsr->time_passed = *(float*)msk_module_get_property_buffer(self, "release") * 44100;
    adsr->gate_on = 0;
}

void msk_adsr_process(MskModule *self, int start, int frames, void *state)
{
    const float * const gate = msk_module_get_input_buffer(self, "gate");
    float attack = *(float*)msk_module_get_property_buffer(self, "attack") * 44100;
    float decay = *(float*)msk_module_get_property_buffer(self, "decay") * 44100;
    float sustain = *(float*)msk_module_get_property_buffer(self, "sustain");
    float release = *(float*)msk_module_get_property_buffer(self, "release") * 44100;
    float * const out = msk_module_get_output_buffer(self, "out");
    MskADSRState *adsr = state;
    int i;
    
    for ( i = start; i < frames + start; i++ )
    {
        if ( gate[i] != adsr->gate_on )
        {
            adsr->time_passed = 0;
            adsr->gate_on = gate[i];
        }
        
        if ( !gate[i] )
        {
            if ( adsr->time_passed < release )
            {
                //g_print("Here (%ld).\n", adsr->time_passed);
                out[i] = adsr->release_value * (1 - (adsr->time_passed / release));
                adsr->time_passed++;
            }
            else
            {
                out[i] = 0;
            }
        }
        else if ( adsr->time_passed < attack )
        {
            out[i] = adsr->time_passed / attack;
            adsr->release_value = out[i];
            adsr->time_passed++;
        }
        else if ( adsr->time_passed - attack < decay )
        {
            out[i] = 1 - (1-sustain)*((adsr->time_passed - attack) / decay);
            adsr->release_value = out[i];
            adsr->time_passed++;
        }
        else
            adsr->release_value = out[i] = sustain;
    }
}

MskModule *msk_adsr_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "ADSR", msk_adsr_process);
    
    msk_add_state(mod, msk_adsr_activate, NULL, sizeof(MskADSRState));
    
    msk_add_input_port(mod, "gate", MSK_AUDIO_DATA, 0.0f);
    msk_add_output_port(mod, "out", MSK_AUDIO_DATA);
    
    msk_add_float_property(mod, "attack", 0.03);
    msk_add_float_property(mod, "decay", 0.05);
    msk_add_float_property(mod, "sustain", 0.6);
    msk_add_float_property(mod, "release", 0.5);
    
    return mod;
}

