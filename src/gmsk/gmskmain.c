#include <glib.h>
#include <cairo.h>
#include <string.h>

#include "msk0/msk0.h"
#include "gmsk.h"
#include "gmskinternal.h"


GMutex *lock_for_model;

MskContainer *root_container, *current_container;

GmskInvalidateCallback invalidate_callback;
GmskSelectModuleCallback select_module_callback;
GmskErrorMessageCallback error_message_callback;
gpointer invalidate_userdata;
gpointer select_module_userdata;
gpointer error_message_userdata;

void gmsk_init(MskContainer *root)
{
    lock_for_model = g_mutex_new();

    root_container = root;
    current_container = root;
}

void gmsk_draw_module_at(MskModule *module, int x, int y)
{
    draw_module(module, x, y);
}

void gmsk_set_invalidate_callback(GmskInvalidateCallback callback, gpointer user_data)
{
    invalidate_callback = callback;
    invalidate_userdata = user_data;
}

void gmsk_set_select_module_callback(GmskSelectModuleCallback callback, gpointer user_data)
{
    select_module_callback = callback;
    select_module_userdata = user_data;
}

void gmsk_set_error_message_callback(GmskErrorMessageCallback callback, gpointer user_data)
{
    error_message_callback = callback;
    error_message_userdata = user_data;
}

void gmsk_lock_mutex()
{
    g_mutex_lock(lock_for_model);
}

void gmsk_unlock_mutex()
{
    g_mutex_unlock(lock_for_model);
}

MskModule *gmsk_create_module(char *name)
{
    MskModule *module;
    GError *error = NULL;

    gmsk_lock_mutex();
    msk_container_deactivate(root_container->module->world->root);

    module = msk_factory_create_module(name, current_container, &error);

    msk_container_activate(root_container->module->world->root);
    gmsk_unlock_mutex();

    if ( error )
    {
        gmsk_error_message(error->message);
        g_error_free(error);
        return NULL;
    }

    draw_module(module, -1, -1);

    /* These modules also create new ports on the parent container. */
    if ( !strcmp(module->name, "input") || !strcmp(module->name, "output") )
    {
        redraw_module(module->parent->module);
    }

    // TODO: Maybe also select it?

    gmsk_invalidate();
    return module;
}

MskContainer *gmsk_create_container(char *name)
{
    MskContainer *container;

    gmsk_lock_mutex();
    msk_container_deactivate(root_container->module->world->root);

    container = msk_factory_create_container(name, current_container);

    msk_container_activate(root_container->module->world->root);
    gmsk_unlock_mutex();

    draw_module(container->module, -1, -1);

    gmsk_invalidate();
    return container;
}

