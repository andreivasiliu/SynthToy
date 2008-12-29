#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"

void msk_message_note_off(MskWorld *world, short note, short velocity);

/* Find the first free voice, or the oldest used voice. */
static int assign_free_voice(MskInstrument *instrument)
{
    int voices = instrument->container->voices;
    int voice;
    
    for ( voice = 0; voice < voices; voice++ )
    {
        if ( !instrument->voice_active[voice] )
        {
            instrument->voice_active[voice] = TRUE;
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


/* Very, very few keyboards support NoteOff velocity... but if it's so easy
 * to implement, why the hell not? */
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
                instrument->voice_velocity[voice] = velocity;
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
    
    mod = msk_module_create(parent, "voiceactive",
                            msk_voiceactive_process,
                            NULL,
                            NULL,
                            0);
    
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
    
    mod = msk_module_create(parent, "voicepitch",
                            msk_voicepitch_process,
                            NULL,
                            NULL,
                            0);
    
    msk_add_output_port(mod, "pitch", MSK_AUDIO_DATA);
    
    return mod;
}

