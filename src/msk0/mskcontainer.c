#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


#define MSK_SIMPLE_CONTAINER 1


// This obviously needs to use a non-null state if the container itself is being
// polyphoned by its parent.

void msk_container_activate(MskModule *self, void *state)
{
    MskContainer *container = self->container;
    GList *item;
    int i = 0;
    
    container->module_states = g_new0(gpointer, g_list_length(container->process_order));
    
    for ( item = container->process_order; item; item = item->next, i++ )
    {
        MskModule *mod = item->data;
        
        if ( mod->state_size )
            container->module_states[i] = g_malloc(mod->state_size);
        else
            container->module_states[i] = NULL;
        
        if ( mod->activate )
            mod->activate(mod, container->module_states[i]);
    }
}


void msk_container_deactivate(MskModule *self, void *state)
{
    
    // ...
    
}


void msk_container_process(MskModule *self, int start, int nframes, void *state)
{
    MskContainer *container = self->container;
    GList *item;
    int i = 0;
    
    for ( item = container->process_order; item; item = item->next, i++ )
    {
        MskModule *mod = item->data;
        
        if ( mod->process )
            mod->process(mod, start, nframes, container->module_states[i]);
    }
}


MskContainer *msk_container_create(MskContainer *parent)
{
    MskContainer *container;
    MskModule *module;
    
    // Create the shell/wrapper/outside/whatever module.
    module = msk_module_create(parent, "container",
                               msk_container_process, // ?
                               msk_container_activate,
                               msk_container_deactivate,
                               0);
    
    container = g_new0(MskContainer, 1);
    container->module = module;
    module->container = container;
    
    msk_add_integer_property(module, "type", MSK_SIMPLE_CONTAINER);
    
    return container;
}


MskModule *msk_input_create(MskContainer *parent, gchar *name, guint type)
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


// This only works on monophonic containers..
MskModule *msk_output_create(MskContainer *parent, gchar *name, guint type)
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


