#include <glib.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


MskModule *msk_module_create(MskContainer *parent, gchar *name,
                             MskProcessCallback process,
                             MskActivateCallback activate,
                             MskDeactivateCallback deactivate,
                             gsize state_size)
{
    MskModule *mod;
    
    mod = g_new0(MskModule, 1);
    mod->name = g_strdup(name);
    mod->parent = parent;
    mod->process = process;
    mod->activate = activate;
    mod->deactivate = deactivate;
    mod->state_size = state_size;
    
    if ( parent )
    {
        mod->world = parent->module->world;
        parent->module_list = g_list_append(parent->module_list, mod);
        parent->module->prepared = FALSE;
        
        // TODO: A simple prepend will do too.
        msk_container_sort(parent);
    }
    
    mod->in_ports = mod->out_ports = mod->properties = NULL;
    
    return mod;
}


void msk_module_destroy(MskModule *mod)
{
    /* Unlink all ports. */
    // TODO
    
    /* Unlink from world. */
    if ( mod->parent )
        mod->parent->module_list = g_list_remove(mod->parent->module_list, mod);
    
    /* Free ports. */
    // TODO
    
    /* Free. */
    g_free(mod->name);
    g_free(mod);
}

void msk_module_activate(MskModule *mod)
{
    int voices = mod->parent->voices * mod->parent->voice_size;
    int v;
    
    g_assert(mod->state == NULL);
    mod->state = g_ptr_array_new();
    
    for ( v = 0; v < voices; v++ )
    {
        void *state;
        
        if ( mod->state_size )
            state = g_malloc(mod->state_size);
        else
            state = NULL;
        
        if ( mod->activate )
            mod->activate(mod, state);
        
        g_ptr_array_add(mod->state, state);
    }
}


void msk_module_deactivate(MskModule *mod)
{
    int voices = mod->parent->voices * mod->parent->voice_size;
    int v;
    
    g_assert(mod->state != NULL);
    
    for ( v = 0; v < voices; v++ )
    {
        void *state = g_ptr_array_index(mod->state, v);
        
        if ( mod->deactivate )
            mod->activate(mod, state);
        
        if ( state )
            g_free(state);
    }
    
    g_ptr_array_free(mod->state, TRUE);
    mod->state = NULL;
}


void msk_add_global_state(MskModule *module,
                          MskGlobalActivateCallback global_activate,
                          MskGlobalDeactivateCallback global_deactivate,
                          gsize state_size)
{
    module->global_state_size = state_size;
    module->global_activate = global_activate;
    module->global_deactivate = global_deactivate;
}


MskPort *msk_module_get_input_port(MskModule *mod, gchar *port_name)
{
    GList *port;
    
    for ( port = mod->in_ports; port; port = port->next )
    {
        if ( !strcmp(((MskPort*)port->data)->name, port_name) )
            return (MskPort*) port->data;
    }
    
    return NULL;
}


MskPort *msk_module_get_output_port(MskModule *mod, gchar *port_name)
{
    GList *port;
    
    for ( port = mod->out_ports; port; port = port->next )
    {
        if ( !strcmp(((MskPort*)port->data)->name, port_name) )
            return (MskPort*) port->data;
    }
    
    return NULL;
}


MskProperty *msk_module_get_property(MskModule *mod, gchar *prop_name)
{
    GList *prop;
    
    for ( prop = mod->properties; prop; prop = prop->next )
    {
        if ( !strcmp(((MskProperty*)prop->data)->name, prop_name) )
            return (MskProperty*) prop->data;
    }
    
    return NULL;
}


void msk_disconnect_input(MskPort *in_port)
{
    MskPort *out_port;
    
    g_assert(in_port != NULL);
    g_assert(in_port->input.connection != NULL);
    
    out_port = in_port->input.connection;
    
    in_port->input.connection = NULL;
    out_port->output.connections = g_list_remove(out_port->output.connections, in_port);
}

// TODO: Find a proper way to do errors.

void msk_connect_ports(MskModule *left, gchar *left_port_name,
                       MskModule *right, gchar *right_port_name)
{
    MskPort *left_port, *right_port;
    
    if ( !left || !right )
    {
        g_error("A NULL module was provided."); // TODO;
    }
    
    left_port = msk_module_get_output_port(left, left_port_name);
    right_port = msk_module_get_input_port(right, right_port_name);
    
    if ( !left_port || !right_port )
    {
        g_error("An inexistent port was specified."); // TODO;
    }
    
    // TODO: Check for same-container condition.
    
    /* No longer possible.
    if ( g_list_find(left_port->output.connections, right_port) )
    {
        g_error("Ports already connected.");
    }
    */
    
    if ( right_port->input.connection )
    {
        MskModule *remote_mod = right_port->input.connection->owner;
        
        msk_disconnect_input(right_port);
        
        // TODO: strcmp sucks
        if ( !strcmp(remote_mod->name, "autoconstant") )
        {
            msk_module_destroy(remote_mod);
        }
    }
    
    left_port->output.connections = g_list_append(left_port->output.connections, right_port);
    right_port->input.connection = left_port;
    
/*    // TODO: use something other than left_port
    while ( left_port->output.hardlink )
        left_port = left_port->output.hardlink;
    
    right_port->buffer = left_port->buffer;*/
    
    // TODO: check if it succeeded
    msk_container_sort(left->parent);
}


void msk_meld_ports(MskPort *inport, MskPort *outport)
{
    g_free(outport->buffer);
    outport->buffer = NULL;
    
    outport->output.hardlink = inport;
}

/*** Ports and properties ***/

MskPort *msk_add_input_port(MskModule *mod, gchar *name, guint type, gfloat default_value)
{
    MskPort *port;
    
    // TODO: check name conflicts, improve type of default_value.
    
    port = g_new0(MskPort, 1);
    
    port->name = g_strdup(name);
    port->port_type = type;
    port->default_value = default_value;
    port->owner = mod;
    
    mod->in_ports = g_list_append(mod->in_ports, port);
    
    if ( mod->parent )
    {
        MskModule *ac;
        
        ac = msk_autoconstant_create(mod->parent);
        msk_module_set_float_property(ac, "value", default_value);
        
        msk_connect_ports(ac, "output", mod, name);
    }
    else
    {
        port->buffer = g_new(float, mod->world->block_size);
    }
    
    return port;
}

MskPort *msk_add_output_port(MskModule *mod, gchar *name, guint type)
{
    MskPort *port;
    
    // TODO: check name conflicts.
    
    port = g_new0(MskPort, 1);
    
    port->name = g_strdup(name);
    port->port_type = type;
    port->owner = mod;
    
    mod->out_ports = g_list_append(mod->out_ports, port);
    
    // TODO: Fix me!
    if ( type == MSK_AUDIO_DATA )
        port->buffer = g_new(float, mod->world->block_size);
    
    return port;
}

MskProperty *msk_add_float_property(MskModule *mod, gchar *name, gfloat value)
{
    MskProperty *property;
    
    property = g_new0(MskProperty, 1);
    property->name = g_strdup(name);
    property->type = MSK_FLOAT_PROPERTY;
    property->owner = mod;
    
    property->value = g_new(gfloat, 1);
    *(gfloat*)property->value = value;
    
    mod->properties = g_list_append(mod->properties, property);
    
    return property;
}

MskProperty *msk_add_integer_property(MskModule *mod, gchar *name, gint value)
{
    MskProperty *property;
    
    property = g_new0(MskProperty, 1);
    property->name = g_strdup(name);
    property->type = MSK_INT_PROPERTY;
    
    property->value = g_new(gint, 1);
    *(gint*)property->value = value;
    
    mod->properties = g_list_append(mod->properties, property);
    
    return property;
}

gconstpointer msk_module_get_input_buffer(MskModule *mod, gchar *name)
{
    MskPort *port = msk_module_get_input_port(mod, name);
    
    if ( !port )
        g_error("Output port '%s' was not found.", name);
    
    port = port->input.connection;
    
    /* Follow hardlinks ((TODO: which are now symlinks)) */
    while ( port->output.hardlink )
    {
        port = port->output.hardlink->input.connection;
    }
    
    return port->buffer;
}


gpointer msk_module_get_output_buffer(MskModule *mod, gchar *name)
{
    MskPort *port = msk_module_get_output_port(mod, name);
    
    if ( !port )
        g_error("Output port '%s' was not found.", name);
    
    /* Follow hardlinks ((TODO: which are now symlinks)) */
    while ( port->output.hardlink )
    {
        port = port->output.hardlink->input.connection;
    }
    
    return port->buffer;
}


gconstpointer msk_module_get_property_buffer(MskModule *mod, gchar *name)
{
    MskProperty *prop = msk_module_get_property(mod, name);
    
    if ( !prop )
        g_error("Property '%s' was not found.", name);
    
    return prop->value;
}


void msk_module_set_float_property(MskModule *mod, gchar *name, gfloat value)
{
    MskProperty *prop = msk_module_get_property(mod, name);
    
    if ( !prop )
        g_error("Property '%s' was not found.", name);
    
    // TODO: Check type.
    *(float*)prop->value = value;
}

