#include <glib.h>
#include <string.h>

#include "msk0.h"
#include "mskinternal.h"


// TODO: the errors used here have no 'domain' or 'code' set to them.. they're both 0.
// That's because I don't know what they should be.. but they probably shouldn't be 0.

MskContainer *msk_load_world_from_file(const gchar *filename,
        MskModuleLoadCallback moduleload_callback, GError **error)
{
    GKeyFile *keyfile;
    int modules_count, containers_count, connections_count;
    MskModule **modules;
    MskContainer **containers;
    MskContainer *world = NULL;
    int abort = FALSE;
    int i;

    keyfile = g_key_file_new();
    g_key_file_load_from_file(keyfile, filename, 0, error);

    if ( *error )
        return FALSE;

    if ( !g_key_file_has_group(keyfile, "mskworld") )
    {
        g_set_error(error, 0, 0, "The file does not appear to be a valid MSK collection.");
        return FALSE;
    }

    modules_count = g_key_file_get_integer(keyfile, "mskworld", "modules", error);
    containers_count = g_key_file_get_integer(keyfile, "mskworld", "containers", error);
    connections_count = g_key_file_get_integer(keyfile, "mskworld", "connections", error);

    if ( *error )
    {
        g_prefix_error(error, "Load error: ");
        g_key_file_free(keyfile);
        return FALSE;
    }

    if ( modules_count < 0 || containers_count < 1 || connections_count < 0 )
    {
        g_set_error(error, 0, 0, "The file does not appear to be a valid MSK "
                "collection (it has invalid module/container/connection counts).");
        g_key_file_free(keyfile);
        return FALSE;
    }

    containers = g_new0(MskContainer *, containers_count);
    modules = g_new0(MskModule *, modules_count);

    for ( i = 0; i < containers_count && !abort; i++ )
    {
        char id[128];
        char *type;
        int parent = 0;

        g_snprintf(id, 128, "container %d", i);

        type = g_key_file_get_string(keyfile, id, "type", error);
        if ( !*error && !!strcmp(type, "world") )
            parent = g_key_file_get_integer(keyfile, id, "parent", error);

        if ( *error )
        {
            g_prefix_error(error, "Load error: ");
            abort = TRUE;
        }

        if ( !*error && !!strcmp(type, "world") && (parent < 0 || parent >= i) )
        {
            g_set_error(error, 0, 0, "Load error: Invalid 'parent' on container %d.", i);
            abort = TRUE;
        }

        if ( abort )
        {
        }
        else if ( !strcmp(type, "container") )
        {
            containers[i] = msk_container_create(containers[parent]);
        }
        else if ( !strcmp(type, "instrument") )
        {
            containers[i] = msk_instrument_create(containers[parent]);
        }
        else if ( !strcmp(type, "world") )
        {
            gulong sample_rate;
            gsize block_size;

            // check if world already exists

            /* Note that these are actually from the 'mskworld' group. */
            sample_rate = g_key_file_get_integer(keyfile, "mskworld", "sample_rate", NULL);
            block_size = g_key_file_get_integer(keyfile, "mskworld", "block_size", NULL);

            // sTODO: anity checks.

            containers[i] = msk_world_create(sample_rate, block_size);
            world = containers[i];
        }
        else
        {
            g_set_error(error, 0, 0, "Load error: Unknown container type: '%s'.", type);
            abort = TRUE;
        }

        g_free(type);

        if ( abort )
            break;
    }

    g_print("Step 3!\n");

    for ( i = 0; i < modules_count && !abort; i++ )
    {
        char id[128];
        char *type;
        int parent;

        g_snprintf(id, 128, "module %d", i);

        // TODO: check existence of group

        type = g_key_file_get_string(keyfile, id, "type", NULL);
        parent = g_key_file_get_integer(keyfile, id, "parent", NULL);

        // TODO: check above for NULLness, make sure parent is sane

        if ( !strcmp(type, "input") || !strcmp(type, "output") )
        {
            guint port_type;
            char *port_name;

            port_type = g_key_file_get_integer(keyfile, id, "port-type", NULL);
            port_name = g_key_file_get_string(keyfile, id, "port-name", NULL);

            // TODO check above

            if ( !strcmp(type, "input") )
                modules[i] = msk_input_create_with_name(containers[parent], port_name, port_type);
            else
                modules[i] = msk_output_create_with_name(containers[parent], port_name, port_type);

            g_free(port_name);
        }
        else
        {
            modules[i] = msk_factory_create_module(type, containers[parent]);

            if ( !modules[i] )
            {
                g_set_error(error, 0, 0, "The module factory does not know about modules of type '%s'.", type);
                abort = TRUE;
            }
        }

        g_free(type);

        if ( abort )
            break;
    }

    for ( i = 0; i < connections_count && !abort; i++ )
    {
        char id[128];
        MskModule *source;
        MskModule *destination;
        char *source_port, *destination_port;

        g_snprintf(id, 128, "connection %d", i);

        // TODO: check existence of group

        if ( g_key_file_has_key(keyfile, id, "source-container", NULL) )
        {
            int s_id = g_key_file_get_integer(keyfile, id, "source-container", NULL);
            source = containers[s_id]->module;
        }
        else
        {
            int s_id = g_key_file_get_integer(keyfile, id, "source-module", NULL);
            source = modules[s_id];
        }

        if ( g_key_file_has_key(keyfile, id, "destination-container", NULL) )
        {
            int d_id = g_key_file_get_integer(keyfile, id, "destination-container", NULL);
            destination = containers[d_id]->module;
        }
        else
        {
            int d_id = g_key_file_get_integer(keyfile, id, "destination-module", NULL);
            destination = modules[d_id];
        }


        // TODO: check sanity of source and destination

        source_port = g_key_file_get_string(keyfile, id, "source-port", NULL);
        destination_port = g_key_file_get_string(keyfile, id, "destination-port", NULL);

        // TODO: also check if it's possible to connect them

        msk_connect_ports(source, source_port, destination, destination_port);

        g_free(source_port);
        g_free(destination_port);

        if ( abort )
            break;
    }

    if ( !abort )
    {
        for ( i = 0; i < modules_count; i++ )
        {
            char id[128];

            g_snprintf(id, 128, "module %d", i);
            moduleload_callback(keyfile, modules[i], id);
        }

        for ( i = 0; i < containers_count; i++ )
        {
            char id[128];

            g_snprintf(id, 128, "container %d", i);
            moduleload_callback(keyfile, containers[i]->module, id);
        }
    }

    g_free(modules);
    g_free(containers);
    g_key_file_free(keyfile);

    if ( abort )
    {
        if (world)
            msk_world_destroy(world);

        return NULL;
    }

    g_print("Awesomeness!\n");
    return world;
}

/* Stick a unique numeric ID to each and every module in the world. */
static void tag_all_modules(MskContainer *container, int *module_id,
                            int *container_id)
{
    GList *item;

    container->module->save_id = (*container_id)++;

    for ( item = container->module_list; item; item = item->next )
    {
        MskModule *module = item->data;

        if ( module->container )
            tag_all_modules(module->container, module_id, container_id);
        else if ( !!strcmp(module->name, "autoconstant") )
            module->save_id = (*module_id)++;
    }
}

static void save_module(GKeyFile *keyfile, MskModule *module, int *connection_id,
        MskModuleSaveCallback modulesave_callback)
{
    GList *item;
    char id[128];

    /* Save the module itself... unless it must be saved by save_container. */
    /* Also ignore auto-constants, because those are created automatically. */
    // TODO: auto-constants must actually be removed... when they are, remove this too.
    if ( module->container == NULL && !!strcmp(module->name, "autoconstant") )
    {
        g_snprintf(id, 128, "module %d", module->save_id);
        g_key_file_set_string(keyfile, id, "type", module->name);
        g_key_file_set_integer(keyfile, id, "parent", module->parent->module->save_id);

        if ( !strcmp(module->name, "input") )
        {
            MskPort *port = (MskPort*) (g_list_first(module->out_ports)->data);

            g_key_file_set_integer(keyfile, id, "port-type", port->port_type);
            g_key_file_set_string(keyfile, id, "port-name", port->name);
        }
        else if ( !strcmp(module->name, "output") )
        {
            MskPort *port = (MskPort*) (g_list_first(module->in_ports)->data);

            g_key_file_set_integer(keyfile, id, "port-type", port->port_type);
            g_key_file_set_string(keyfile, id, "port-name", port->name);
        }

        modulesave_callback(keyfile, module, id);
    }

    for ( item = module->in_ports; item; item = item->next )
    {
        MskPort *destport = item->data;
        MskPort *srcport = destport->input.connection;

        if ( srcport && !!strcmp(srcport->owner->name, "autoconstant") )
        {
            const char *srctype = srcport->owner->container ? "source-container" : "source-module";
            const char *desttype = module->container ? "destination-container" : "destination-module";
            char conn_id[128];

            g_snprintf(conn_id, 128, "connection %d", (*connection_id)++);

            g_key_file_set_integer(keyfile, conn_id, srctype, srcport->owner->save_id);
            g_key_file_set_integer(keyfile, conn_id, desttype, module->save_id);
            g_key_file_set_string(keyfile, conn_id, "source-port", srcport->name);
            g_key_file_set_string(keyfile, conn_id, "destination-port", destport->name);
        }
    }
}


static void save_container(GKeyFile *keyfile, MskContainer *container, int *connection_id,
        MskModuleSaveCallback modulesave_callback)
{
    char id[128];
    GList *item;

    g_snprintf(id, 128, "container %d", container->module->save_id);

    if ( container == container->module->world->root )
        g_key_file_set_string(keyfile, id, "type", "world");
    else
    {
        if ( container->instrument )
            g_key_file_set_string(keyfile, id, "type", "instrument");
        else
            g_key_file_set_string(keyfile, id, "type", "container");

        g_key_file_set_integer(keyfile, id, "parent", container->module->parent->module->save_id);
    }

    modulesave_callback(keyfile, container->module, id);

    for ( item = container->module_list; item; item = item->next )
    {
        MskModule *module = item->data;

        if ( module->container )
            save_container(keyfile, module->container, connection_id, modulesave_callback);

        save_module(keyfile, module, connection_id, modulesave_callback);
    }
}


gboolean msk_save_world_to_file(MskContainer *container, const gchar *filename,
        MskModuleSaveCallback modulesave_callback, GError **error)
{
    GKeyFile *keyfile = g_key_file_new();
    int module_id = 0, container_id = 0, connection_id = 0;
    char *keyfile_string;
    MskWorld *world = container->module->world;

    g_key_file_set_integer(keyfile, "mskworld", "sample_rate", world->sample_rate);
    g_key_file_set_integer(keyfile, "mskworld", "block_size", world->block_size);

    tag_all_modules(world->root, &module_id, &container_id);

    g_key_file_set_integer(keyfile, "mskworld", "modules", module_id);
    g_key_file_set_integer(keyfile, "mskworld", "containers", container_id);

    save_container(keyfile, world->root, &connection_id, modulesave_callback);

    g_key_file_set_integer(keyfile, "mskworld", "connections", connection_id);

    keyfile_string = g_key_file_to_data(keyfile, NULL, NULL);
    g_file_set_contents(filename, keyfile_string, -1, error);
    g_key_file_free(keyfile);

    if (*error)
        return FALSE;

    return TRUE;
}
