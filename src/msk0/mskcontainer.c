#include <glib.h>
#include <string.h> // for memset()

#include "msk0.h"
#include "mskinternal.h"


#define MSK_SIMPLE_CONTAINER 1
#define MSK_INSTRUMENT_CONTAINER 2

void msk_container_activate(MskContainer *self)
{
    GList *item;
    
    for ( item = self->module_list; item; item = item->next )
    {
        MskModule *mod = item->data;
        
        msk_module_activate(mod);
        
        if ( mod->container )
            msk_container_activate(mod->container);
    }
}


void msk_container_deactivate(MskContainer *self)
{
    GList *item;
    
    for ( item = self->module_list; item; item = item->next )
    {
        MskModule *mod = item->data;
        
        msk_module_deactivate(mod);
        
        if ( mod->container )
            msk_container_deactivate(mod->container);
    }
}


void msk_container_process(MskContainer *self, int start, int nframes, guint voice)
{
    GList *item;
    int v;
    
    // TODO: This must go away; find a better way to initialize those buffers.
    for ( item = self->module->out_ports; item; item = item->next )
    {
        MskPort *port = item->data;
        
        memset(port->buffer, 0, self->module->world->block_size*sizeof(float));
    }
    
    for ( v = 0; v < self->voices; v++ )
    {
        self->current_voice = v;
        
        for ( item = self->process_order; item; item = item->next )
        {
            MskModule *mod = item->data;
            
            if ( mod->process )
            {
                void *state = g_ptr_array_index(mod->state, voice * self->voice_size + v);
                mod->process(mod, start, nframes, state);
            }
            
            if ( mod->container )
                msk_container_process(mod->container, start, nframes, v);
        }
    }
}


MskContainer *msk_container_create(MskContainer *parent)
{
    MskContainer *container;
    MskModule *module;
    
    // Create the shell/wrapper/outside/whatever module.
    module = msk_module_create(parent, "container",
                               NULL,
//                               msk_container_process, // ?
                               NULL,
                               NULL,
                               0);
    
    container = g_new0(MskContainer, 1);
    container->module = module;
    container->voices = 1;
    module->container = container;
    
    if ( parent )
        container->voice_size = parent->voices * parent->voice_size;
    else
        container->voice_size = 1;
    
    msk_add_integer_property(module, "type", MSK_SIMPLE_CONTAINER);
    // needs at least one more property: voices
    
    return container;
}


void msk_sort_module(MskModule *mod, GList **process_order)
{
    GList *lport;
    
    for ( lport = mod->in_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        MskModule *linked_mod;
        
        if ( !port->input.connection )
            continue;
        
        linked_mod = port->input.connection->owner;
        
        if ( !linked_mod->prepared )
            msk_sort_module(linked_mod, process_order);
    }
    
    // TODO: change
    *process_order = g_list_append(*process_order, mod);
    mod->prepared = TRUE;
}

/* Attempt to do a topological sort. */
gboolean msk_container_sort(MskContainer *container)
{
    GList *process_order = NULL;
    GList *lmod;
    
    for ( lmod = container->module_list; lmod; lmod = lmod->next )
        ((MskModule *)lmod->data)->prepared = FALSE;
    
    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        MskModule *mod = lmod->data;
        
        if ( !mod->prepared )
            msk_sort_module(mod, &process_order);
    }
    
    if ( container->process_order )
        g_list_free(container->process_order);
    container->process_order = process_order;
    return TRUE;
}


MskModule *msk_input_create_with_name(MskContainer *parent, gchar *name, guint type)
{
    MskModule *mod;
    MskPort *interior_port;
    MskPort *exterior_port;
    
    mod = msk_module_create(parent, "input",
                            NULL,
                            NULL,
                            NULL,
                            0);
    
    interior_port = msk_add_output_port(mod, name, type);
    exterior_port = msk_add_input_port(parent->module, name, type, 0.0f);  // ?
    
    msk_meld_ports(exterior_port, interior_port);
    
    return mod;
}

MskModule *msk_input_create(MskContainer *parent)
{
    return msk_input_create_with_name(parent, "in", MSK_AUDIO_DATA);
}

void msk_output_process(MskModule *self, int start, int frames, void *state)
{
    // TODO: FIX THIIIS!
    char *name = ((MskPort*)self->in_ports->data)->name;
    const float * const in = msk_module_get_input_buffer(self, name);
    float * const mix_to = self->mix_to->buffer;
    int i;
    
    for ( i = start; i < start + frames; i++ )
        mix_to[i] += in[i];
}

MskModule *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type)
{
    MskModule *mod;
    MskPort *interior_port;
    MskPort *exterior_port;
    
    mod = msk_module_create(parent, "output",
                            msk_output_process,
                            NULL,
                            NULL,
                            0);
    
    interior_port = msk_add_input_port(mod, name, type, 0.0f);  // ?
    exterior_port = msk_add_output_port(parent->module, name, type);
    
    mod->mix_to = exterior_port;
    
    // This only works on monophonic containers..
    //msk_meld_ports(interior_port, exterior_port);
    
    return mod;
}

MskModule *msk_output_create(MskContainer *parent)
{
    return msk_output_create_with_name(parent, "out", MSK_AUDIO_DATA);
}


void msk_voice_process(MskModule *self, int start, int frames, void *state)
{
    float * const out = msk_module_get_output_buffer(self, "nr");
    int i;
    const int voice = self->parent->current_voice;
    
    for ( i = start; i < start + frames; i++ )
        out[i] = voice;
}

/* Output the number of the current voice. */
MskModule *msk_voice_create(MskContainer *parent)
{
    MskModule *mod;
    
    mod = msk_module_create(parent, "voice",
                            msk_voice_process,
                            NULL,
                            NULL,
                            0);
    
    msk_add_output_port(mod, "nr", MSK_AUDIO_DATA);
    
    return mod;
}


// This is almost identical with msk_container_create... and that's a
// very big problem.
MskContainer *msk_instrument_create(MskContainer *parent)
{
    MskWorld *world;
    MskContainer *container;
    MskInstrument *instrument;
    MskModule *module;
    
    // Create the shell/wrapper/outside/whatever module.
    module = msk_module_create(parent, "instrument",
                               NULL,
//                               msk_container_process, // ?
                               NULL,
                               NULL,
                               0);
    
    container = g_new0(MskContainer, 1);
    container->module = module;
    container->voices = 4;
    module->container = container;
    
    instrument = g_new0(MskInstrument, 1);
    instrument->container = container;
    instrument->voice_active = g_new0(char, container->voices);
    instrument->voice_note = g_new0(short, container->voices);
    instrument->voice_velocity = g_new0(short, container->voices);
    container->instrument = instrument;
    
    // Add to the world
    world = module->world;
    world->instruments = g_list_prepend(world->instruments, instrument);
    
    
    if ( parent )
        container->voice_size = parent->voices * parent->voice_size;
    else
        container->voice_size = 1;
    
    msk_add_integer_property(module, "type", MSK_INSTRUMENT_CONTAINER);
    // needs at least one more property: voices
    
    return container;
}
