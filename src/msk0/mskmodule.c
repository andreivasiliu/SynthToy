#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include <math.h> // for log10()

#include "msk0.h"
#include "mskinternal.h"


MskModule *msk_module_create(MskContainer *parent, gchar *name,
                             MskProcessCallback process)
{
    MskModule *mod;

    mod = g_new0(MskModule, 1);
    mod->name = g_strdup(name);
    mod->parent = parent;
    mod->process = process;

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
    GList *item;

    /* Unlink all ports. */
    for ( item = mod->in_ports; item; item = item->next )
    {
        MskPort *input_port = item->data;

        if ( input_port->input.connection )
            msk_disconnect_input_port(input_port);
    }
    for ( item = mod->in_ports; item; item = item->next )
    {
        MskPort *output_port = item->data;

        msk_disconnect_output_port(output_port);
    }

    /* Unlink from world. */
    if ( mod->parent )
    {
        mod->parent->module_list = g_list_remove(mod->parent->module_list, mod);
        msk_container_sort(mod->parent);
    }

    /* Clear its state, if any. */
    if ( mod->state )
        msk_module_deactivate(mod);

    /* Free ports. */
    // TODO

    /* Free properties. */
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


void msk_add_state(MskModule *module,
                   MskActivateCallback activate,
                   MskDeactivateCallback deactivate,
                   gsize state_size)
{
    module->activate = activate;
    module->deactivate = deactivate;
    module->state_size = state_size;
}


void msk_dynamic_ports(MskModule *module,
                       MskDynamicPortAddCallback dynamic_port_add,
                       MskDynamicPortRemoveCallback dynamic_port_remove)
{
    module->dynamic_port_add = dynamic_port_add;
    module->dynamic_port_remove = dynamic_port_remove;
}


void msk_module_dynamic_port_add(MskModule *module)
{
    if ( module->dynamic_port_add )
        module->dynamic_port_add(module);
}


void msk_module_dynamic_port_remove(MskModule *module)
{
    if ( module->dynamic_port_remove )
        module->dynamic_port_remove(module);
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


static void *create_port_buffer(guint type, MskWorld *world)
{
    void *buffer;

    if ( type == MSK_AUDIO_DATA )
        buffer = g_new(float, world->block_size);
    else if ( type == MSK_CONTROL_DATA )
        buffer = g_new(float, 1);
    else
        g_error("Unknown port type (%d)", type);

    return buffer;
}


void msk_disconnect_input_port(MskPort *in_port)
{
    MskPort *out_port;

    g_assert(in_port != NULL);
    g_assert(in_port->input.connection != NULL);

    out_port = in_port->input.connection;

    in_port->input.connection = NULL;
    out_port->output.connections = g_list_remove(out_port->output.connections, in_port);

    if ( !in_port->buffer )
        in_port->buffer = create_port_buffer(in_port->port_type, in_port->owner->world);

    msk_container_sort(in_port->owner->parent);
}


void msk_disconnect_output_port(MskPort *out_port)
{
    MskPort *in_port;

    while ( out_port->output.connections )
    {
        in_port = g_list_first(out_port->output.connections)->data;

        msk_disconnect_input_port(in_port);
    }
}


// TODO: the error's 'domain' and 'code' should probably not be 0. Find
// proper values for it.

static gboolean can_connect_ports(MskModule *left, gchar *left_port_name,
                                  MskModule *right, gchar *right_port_name,
                                  GError **error,
                                  MskPort **left_port, MskPort **right_port)
{
    if ( !left || !right )
    {
        g_set_error(error, 0, 0, "The %s module is null.",
                !left ? "left" : "right");
        return FALSE;
    }

    if ( !left_port_name || !right_port_name )
    {
        g_set_error(error, 0, 0, "The %s port name is null.",
                !left_port_name ? "left" : "right");
        return FALSE;
    }

    /* The two modules must be in the same container. Also, the world module
     * must not be connected to anything. */
    if ( !left->parent || left->parent != right->parent )
    {
        g_set_error(error, 0, 0, "Both modules must be in the same container.");
        return FALSE;
    }

    *left_port = msk_module_get_output_port(left, left_port_name);
    *right_port = msk_module_get_input_port(right, right_port_name);

    if ( !*left_port || !*right_port )
    {
        g_set_error(error, 0, 0, "An inexistent %s port name (%s) was specified.",
                (!*left_port ? "left" : "right"),
                (!*left_port ? left_port_name : right_port_name));
        return FALSE;
    }

    /* Check if their types are compatible. */
    if ( (*left_port)->port_type != (*right_port)->port_type &&
         msk_get_adapter((*left_port)->port_type, (*right_port)->port_type) == NULL )
    {
        g_set_error(error, 0, 0, "The port types are not compatible, and no "
                "adapter found.");
        return FALSE;
    }

    // TODO: Check if it causes loops.

    return TRUE;
}

gboolean msk_can_connect_ports(MskModule *left, gchar *left_port_name,
                               MskModule *right, gchar *right_port_name,
                               GError **error)
{
    MskPort *dummy_left_port, *dummy_right_port;

    return can_connect_ports(left, left_port_name, right, right_port_name,
            error, &dummy_left_port, &dummy_right_port);
}

void msk_connect_ports(MskModule *left, gchar *left_port_name,
                       MskModule *right, gchar *right_port_name)
{
    MskPort *left_port, *right_port;
    GError *error = NULL;

    can_connect_ports(left, left_port_name, right, right_port_name,
            &error, &left_port, &right_port);

    if ( error )
    {
        g_error("Code bug! Cannot connect ports: %s", error->message);
    }

    if ( right_port->input.connection )
    {
        MskPort *old_left_port = right_port->input.connection;

        right_port->input.connection = NULL;
        old_left_port->output.connections =
            g_list_remove(old_left_port->output.connections, right_port);
    }

    left_port->output.connections = g_list_append(left_port->output.connections, right_port);
    right_port->input.connection = left_port;

    /* Does it need an adapter? If so, then it also needs a separate buffer. */
    if ( left_port->port_type != right_port->port_type )
    {
        if ( !right_port->buffer )
            right_port->buffer = create_port_buffer(right_port->port_type, right->world);
    }
    else if ( right_port->buffer )
    {
        g_free(right_port->buffer);
        right_port->buffer = NULL;
    }

/*    // TODO: use something other than left_port
    while ( left_port->output.hardlink )
        left_port = left_port->output.hardlink;

    right_port->buffer = left_port->buffer;*/

    /* If this fails, then it's a bug in the code. The 'can_connect_ports'
     * function should catch everything. */
    msk_container_sort(left->parent);
}

void msk_meld_ports(MskPort *inport, MskPort *outport)
{
    g_free(outport->buffer);
    outport->buffer = NULL;

    outport->output.hardlink = inport;
}

/*** Ports and properties ***/

static int port_name_is_unique(MskModule *mod, const char *name)
{
    GList *item;

    for ( item = mod->in_ports; item; item = item->next )
    {
        MskPort *port = item->data;

        if ( !strcmp(port->name, name) )
            return FALSE;
    }

    for ( item = mod->out_ports; item; item = item->next )
    {
        MskPort *port = item->data;

        if ( !strcmp(port->name, name) )
            return FALSE;
    }

    return TRUE;
}


static char *make_port_name(MskModule *mod, const char *name)
{
    if ( !name || !name[0] )
        g_error("Null port name given when constructing module '%s'.",
                mod->name);

    /* If the last character is a #, replace it with a number. */
    if ( name[strlen(name)-1] == '#' )
    {
        int number = 1;

        while ( 1 )
        {
            /* Lengths:
             *   - name without leading '#': strlen(name) - 1
             *   - number: (int)log10(number) + 1
             *   - null terminator: 1
             */
            char unique_name[strlen(name) + (int)log10(number) + 1];

            memcpy(unique_name, name, strlen(name)-1);
            g_sprintf(unique_name + strlen(name)-1, "%d", number);

            if ( port_name_is_unique(mod, unique_name) )
                return g_strdup(unique_name);

            number++;
        }
    }
    else
    {
        if ( !port_name_is_unique(mod, name) )
            g_error("Duplicate port name (%s) given when constructing "
                    "module '%s'.", name, mod->name);

        return g_strdup(name);
    }
}

MskPort *msk_add_input_port(MskModule *mod, gchar *name, guint type, gfloat default_value)
{
    MskPort *port;

    // TODO: improve type of default_value.

    port = g_new0(MskPort, 1);

    port->name = make_port_name(mod, name);
    port->port_type = type;
    port->default_value = default_value;
    port->owner = mod;

    mod->in_ports = g_list_append(mod->in_ports, port);

/*    if ( mod->parent )
    {
        MskModule *ac;

        // This must go away. It's easy, it works, but it's an ugly workaround.
        ac = msk_autoconstant_create(mod->parent);
        msk_module_set_float_property(ac, "value", default_value);

        msk_connect_ports(ac, "output", mod, port->name);
    }
    else*/
    {
        port->buffer = create_port_buffer(port->port_type, mod->world);
    }

    msk_container_sort(mod->parent);

    return port;
}


/* Currently only removes the last port. */
void msk_remove_input_port(MskModule *mod)
{
    GList *last_item;
    MskPort *in_port;

    last_item = g_list_last(mod->in_ports);

    if ( !last_item )
        g_error("No input ports available to remove on module '%s'.",
                mod->name);


    /* Unlink. */
    in_port = last_item->data;
    mod->in_ports = g_list_delete_link(mod->in_ports, last_item);
    if ( in_port->input.connection )
    {
        MskPort *out_port = in_port->input.connection;

        //msk_disconnect_input_port(in_port);
        in_port->input.connection = NULL;
        out_port->output.connections = g_list_remove(out_port->output.connections, in_port);
        msk_container_sort(mod->parent);
    }

    /* Free. */
    g_free(in_port->name);
    if ( in_port->buffer )
        g_free(in_port->buffer);
    g_free(in_port);
}

MskPort *msk_add_output_port(MskModule *mod, gchar *name, guint type)
{
    MskPort *port;

    port = g_new0(MskPort, 1);

    port->name = make_port_name(mod, name);
    port->port_type = type;
    port->owner = mod;

    mod->out_ports = g_list_append(mod->out_ports, port);
    port->buffer = create_port_buffer(type, mod->world);

    return port;
}

static MskProperty *add_property(MskModule *mod, gchar *name)
{
    MskProperty *property;

    property = g_new0(MskProperty, 1);
    property->name = g_strdup(name);
    property->owner = mod;

    mod->properties = g_list_append(mod->properties, property);

    return property;
}

MskProperty *msk_add_float_property(MskModule *mod, gchar *name, gfloat value)
{
    MskProperty *property = add_property(mod, name);

    property->type = MSK_FLOAT_PROPERTY;
    property->value = g_new(gfloat, 1);
    *(gfloat*)property->value = value;

    return property;
}

MskProperty *msk_add_integer_property(MskModule *mod, gchar *name, gint value)
{
    MskProperty *property = add_property(mod, name);

    property->type = MSK_INT_PROPERTY;
    property->value = g_new(gint, 1);
    *(gint*)property->value = value;

    return property;
}

MskProperty *msk_add_string_property(MskModule *mod, gchar *name, gchar *value)
{
    MskProperty *property = add_property(mod, name);

    property->type = MSK_STRING_PROPERTY;
    property->value = g_strdup(value);

    return property;
}

void msk_property_add_write_callback(MskProperty *property, MskPropertyWriteCallback callback)
{
    property->callback = callback;
}

gconstpointer msk_module_get_input_buffer(MskModule *mod, gchar *name)
{
    MskPort *input_port = msk_module_get_input_port(mod, name);
    MskPort *output_port;

    if ( !input_port )
        g_error("Output port '%s' was not found.", name);

    /* This is confusing, isn't it? That's what you get for having so many
     * cases. */

    /* Follow connections and hardlinks ((TODO: which are now symlinks)) */
    while ( TRUE )
    {
        if ( input_port->buffer )
            return input_port->buffer;
        output_port = input_port->input.connection;

        if ( !output_port->output.hardlink )
            return output_port->buffer;
        input_port = output_port->output.hardlink;
    }
}


gpointer msk_module_get_output_buffer(MskModule *mod, gchar *name)
{
    MskPort *output_port = msk_module_get_output_port(mod, name);
    MskPort *input_port;

    if ( !output_port )
        g_error("Output port '%s' was not found.", name);

    /* Follow connections and hardlinks ((TODO: which are now symlinks)) */
    while ( TRUE )
    {
        if ( !output_port->output.hardlink )
            return output_port->buffer;
        input_port = output_port->output.hardlink;

        if ( input_port->buffer )
            return input_port->buffer;
        output_port = input_port->input.connection;
    }
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
    if ( prop->callback )
        prop->callback(prop, &value);
    *(float*)prop->value = value;
}

void msk_property_set_value_from_string(MskProperty *property, gchar *value)
{
    // TODO: scanf isn't good.

    if ( property->type == MSK_INT_PROPERTY )
    {
        int newvalue;

        sscanf(value, "%d", &newvalue);
        if ( property->callback )
            property->callback(property, &newvalue);
        *(int*)property->value = newvalue;
    }
    else if ( property->type == MSK_FLOAT_PROPERTY )
    {
        float newvalue;

        sscanf(value, "%f", &newvalue);
        if ( property->callback )
            property->callback(property, &newvalue);
        *(float*)property->value = newvalue;
    }
    else if ( property->type == MSK_STRING_PROPERTY )
    {
        if ( property->value )
            g_free(property->value);
        property->value = g_strdup(value);
        // TODO: callback.
    }
}

gchar *msk_property_get_value_as_string(MskProperty *property)
{
    char *string_value;

    g_assert(property != NULL);

    if ( property->type == MSK_INT_PROPERTY )
        string_value = g_strdup_printf("%d", *(int *) property->value);
    else if ( property->type == MSK_FLOAT_PROPERTY )
        string_value = g_strdup_printf("%f", *(float *) property->value);
    else if ( property->type == MSK_STRING_PROPERTY )
        string_value = g_strdup((char *) property->value);
    else
        g_error("Unkown property type.");

    return string_value;
}

