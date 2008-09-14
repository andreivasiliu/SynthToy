#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/** This create a world, puts a container into it, and returns the container.
 * 
 * @param sample_rate The number of samples in a second (usually 44100 Hz).
 * @return A container inside the newly created world.
 */

MskContainer *msk_world_create(gulong sample_rate, gsize block_size)
{
    MskWorld *world;
    
    world = g_new0(MskWorld, 1);
    world->sample_rate = sample_rate;
    world->block_size = block_size;
    world->root = msk_container_create(NULL);
    world->root->module->world = world;
    
    return world->root;
}


void do_a_dfs(MskModule *mod, GList **process_order)
{
    GList *lport, *lconn;
    
    // TODO: Move it.
    extern void msk_prepare_container(MskContainer *container);
    
    if ( mod->container )
        msk_prepare_container(mod->container);
    
    for ( lport = mod->out_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        
        for ( lconn = port->output.connections; lconn; lconn = lconn->next )
        {
            MskModule *linked_mod = ((MskPort*) lconn->data)->owner;
            
            if ( linked_mod->prepared )
                continue;
            
            if ( linked_mod->parent != mod->parent )
            {
                g_print("Wha?\n");
                msk_prepare_container(linked_mod->parent);
            }
            else
                do_a_dfs(linked_mod, process_order);
        }
    }
    
    *process_order = g_list_prepend(*process_order, mod);
    mod->prepared = TRUE;
}


void msk_prepare_container(MskContainer *container)
{
    GList *lmod;
    
    if ( container->process_order )
        g_list_free(container->process_order);
    
    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        MskModule *mod = (MskModule*) lmod->data;
        
        if ( !mod->prepared )
            do_a_dfs(mod, &container->process_order);
    }
}


void msk_create_buffers_on_module(MskModule *mod)
{
    // TODO: move.
    void msk_create_buffers_on_container(MskContainer *container);
    
    GList *lport;
    
    g_print("Module: %s.\n", mod->name);
    
    for ( lport = mod->in_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        MskPort *linked_port = port->input.connection;
        
        if ( linked_port )
        {
            g_print("Connecting '%s' (%s) to the buffer of '%s' (%s).\n",
                    port->name, port->owner->name,
                    linked_port->name, linked_port->owner->name);
            port->buffer = linked_port->buffer;
        }
        else
        {
            g_print("Creating empty input for port '%s' (%s).\n",
                    port->name, port->owner->name);
            
            port->buffer = g_new0(float, mod->world->block_size);
            
            // TODO: ** FIX!! **
            {
                int i;
                for ( i = 0; i < mod->world->block_size; i++ )
                    ((float*)port->buffer)[i] = port->default_value;
            }
        }
    }
    
    if ( mod->container )
        msk_create_buffers_on_container(mod->container);
    
    for ( lport = mod->out_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        
        if ( port->output.hardlink )
            port->buffer = port->output.hardlink->buffer;
        else
            port->buffer = g_new0(float, mod->world->block_size);
    }
}

void msk_create_buffers_on_container(MskContainer *container)
{
    GList *lmod;
    
    g_print("Entering container.\n");
    
    for ( lmod = container->process_order; lmod; lmod = lmod->next )
    {
        MskModule *mod = lmod->data;
        
        msk_create_buffers_on_module(mod);
    }
    
    g_print("Leaving container.\n");
}


void list_order(GList *items, gint indent)
{
    GList *item;
    
    for ( item = items; item; item = item->next )
    {
        MskModule *mod = item->data;
        
        g_print(" %*s- %s\n", indent, "", mod->name);
        if ( mod->container )
            list_order(mod->container->process_order, indent + 2);
    }
    
}

void msk_world_prepare(MskContainer *container)
{
    msk_prepare_container(container);
    msk_create_buffers_on_module(container->module);
    
    list_order(container->process_order, 0);
    
    container->module->activate(container->module, NULL);
}

void msk_world_run(MskContainer *container)
{
    container->module->process(container->module, 0, container->module->world->block_size, NULL);
    // ..
    
}

