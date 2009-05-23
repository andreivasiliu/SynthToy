#include <glib.h>
#include <cairo.h>

#include "msk0/msk0.h"
#include "gmsk.h"
#include "gmskinternal.h"


static void load_callback(GKeyFile *keyfile, MskModule *module, char *id)
{
    int x, y;

    /* Ignore the 'world' container. We don't draw that. */
    if ( module->container && module->container == module->world->root )
        return;

    // TODO handle errors.
    x = g_key_file_get_integer(keyfile, id, "x", NULL);
    y = g_key_file_get_integer(keyfile, id, "y", NULL);

    draw_module(module, x, y);
}


gboolean gmsk_load_world_from_file(const gchar *filename, GError **error)
{
    MskContainer *world;

    world = msk_load_world_from_file(filename, &load_callback, error);

    if ( *error )
        return FALSE;

    gmsk_lock_mutex();

    root_container = world;
    current_container = root_container;
    msk_world_prepare(world);

    gmsk_unlock_mutex();

    g_print("Awesomeness!\n");
    return TRUE;
}


static void save_callback(GKeyFile *keyfile, MskModule *module, char *id)
{
    GraphicalModule *gmod;

    /* Ignore the 'world' container. We don't draw that. */
    if ( module->container && module->container == module->world->root )
        return;

    gmod = find_gmod(module);
    g_assert(gmod != NULL);

    g_key_file_set_integer(keyfile, id, "x", gmod->x);
    g_key_file_set_integer(keyfile, id, "y", gmod->y);
}


gboolean gmsk_save_world_to_file(const gchar *filename, GError **error)
{
    msk_save_world_to_file(root_container, filename, &save_callback, error);

    if (*error)
        return FALSE;

    return TRUE;
}
