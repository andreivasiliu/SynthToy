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
    
    world->lock_for_model = g_mutex_new();
    
    return world->root;
}


void msk_prepare_module(MskModule *mod, GList **process_order)
{
    GList *lport, *lconn;
    
    // TODO: Move it.
    extern void msk_prepare_container(MskContainer *container);
    
    /* Don't recurse further down if it was already prepared. */
    if ( mod->container && !mod->prepared )
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
                msk_prepare_module(linked_mod, process_order);
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
            msk_prepare_module(mod, &container->process_order);
    }
}


void print_list_order(GList *items, gint indent)
{
    GList *item;
    
    for ( item = items; item; item = item->next )
    {
        MskModule *mod = item->data;
        
        g_print(" %*s- %s\n", indent, "", mod->name);
        if ( mod->container )
            print_list_order(mod->container->process_order, indent + 2);
    }
    
}

void msk_world_prepare(MskContainer *container)
{
    print_list_order(container->process_order, 0);
    
    msk_container_activate(container);
}

void msk_world_run(MskContainer *container)
{
    g_mutex_lock(container->module->world->lock_for_model);
    
    msk_container_process(container, 0, container->module->world->block_size, 0);
    //container->module->process(container->module, 0, container->module->world->block_size, NULL);
    // ..
    
    g_mutex_unlock(container->module->world->lock_for_model);
}

void msk_unprepare_container(MskContainer *container)
{
    GList *lmod;
    
    if ( container->process_order )
        g_list_free(container->process_order);
    
    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        MskModule *mod = (MskModule*) lmod->data;
        
        if ( mod->container )
            msk_unprepare_container(mod->container);
        
        mod->prepared = FALSE;
    }
}


void msk_world_unprepare(MskContainer *container)
{
    container->module->deactivate(container->module, NULL);
   //// 
    msk_unprepare_container(container);
}

