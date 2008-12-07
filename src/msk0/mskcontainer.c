#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


#define MSK_SIMPLE_CONTAINER 1


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
//    MskContainer *container = self->container;
    GList *item;
    int v;
    
    for ( v = 0; v < self->voices; v++ )
    {
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

// This only works on monophonic containers..
MskModule *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type)
{
    MskModule *mod;
    MskPort *interior_port;
    MskPort *exterior_port;
    
    mod = msk_module_create(parent, "output",
                            NULL,
                            NULL,
                            NULL,
                            0);
    
    interior_port = msk_add_input_port(mod, name, type, 0.0f);  // ?
    exterior_port = msk_add_output_port(parent->module, name, type);
    
    msk_meld_ports(interior_port, exterior_port);
    
    return mod;
}

MskModule *msk_output_create(MskContainer *parent)
{
    return msk_output_create_with_name(parent, "out", MSK_AUDIO_DATA);
}
