#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


/** This creates a world, puts a container into it, and returns the container.
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

    //world->lock_for_model = g_mutex_new();

    return world->root;
}

// TODO: Quite obvious what to do, right?
void msk_world_destroy(MskContainer *world)
{


}

void msk_world_prepare(MskContainer *container)
{
    g_assert(container == container->module->world->root);

    msk_container_activate(container);
}

void msk_world_run(MskContainer *container)
{
    g_assert(container == container->module->world->root);

    msk_container_process(container, 0, container->module->world->block_size, 0);
}

void msk_world_unprepare(MskContainer *container)
{
    g_assert(container == container->module->world->root);

    msk_container_deactivate(container);
}

