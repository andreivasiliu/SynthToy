#include <glib.h>
#include <string.h> // for memset()

#include "msk0.h"
#include "mskinternal.h"


#define MSK_SIMPLE_CONTAINER 1
#define MSK_INSTRUMENT_CONTAINER 2


void msk_container_activate(MskContainer *self)
{
    MskContainer *parent = self->module->parent;
    GList *item;

    if ( parent )
        self->voice_size = parent->voices * parent->voice_size;
    else
        self->voice_size = 1;

    g_print("Voice size: %d.\n", self->voice_size);

    for ( item = self->module_list; item; item = item->next )
    {
        MskModule *mod = item->data;

        msk_module_activate(mod);
    }
}


void msk_container_deactivate(MskContainer *self)
{
    GList *item;

    for ( item = self->module_list; item; item = item->next )
    {
        MskModule *mod = item->data;

        msk_module_deactivate(mod);
    }
}


void msk_container_process(MskContainer *self, int start, int nframes, guint voice)
{
    GList *item;
    int v;

    if ( self->block_size_limit && self->block_size_limit < nframes )
    {
        gsize processed_frames = 0;

        while ( processed_frames != nframes )
        {
            gsize new_nframes;

            if ( nframes - processed_frames > self->block_size_limit )
                new_nframes = self->block_size_limit;
            else
                new_nframes = nframes - processed_frames;

            msk_container_process(self, start + processed_frames, new_nframes, voice);
            processed_frames += new_nframes;
        }

        return;
    }

    // TODO: This must go away; find a better way to initialize those buffers.
    for ( item = self->module->out_ports; item; item = item->next )
    {
        MskPort *port = item->data;

        memset(port->buffer + start * sizeof(float), 0, nframes * sizeof(float));
    }

    for ( v = 0; v < self->voices; v++ )
    {
        self->current_voice = v;

        for ( item = self->processing_list; item; item = item->next )
        {
            MskProcessor *task = item->data;

            if ( task->type == MSK_PROCESSOR )
            {
                MskModule *mod = task->processor.module;

                if ( mod->process )
                {
                    //void *state = g_ptr_array_index(mod->state, voice * self->voice_size + v);
                    void *state = g_ptr_array_index(mod->state, v * self->voice_size + voice);
                    mod->process(mod, start, nframes, state);
                }

                if ( mod->container )
                    msk_container_process(mod->container, start, nframes, v);
            }
            if ( task->type == MSK_HALFPROCESSOR )
            {
                MskModule *mod = task->halfprocessor.module;
                MskProcessCallback process;

                if ( task->halfprocessor.input )
                    process = mod->process_input;
                else
                    process = mod->process_output;

                if ( process )
                {
                    //void *state = g_ptr_array_index(mod->state, voice * self->voice_size + v);
                    void *state = g_ptr_array_index(mod->state, v * self->voice_size + voice);
                    process(mod, start, nframes, state);
                }
            }
            else if ( task->type == MSK_ADAPTER )
            {
                MskPort *input_port = task->adapter.input_port;
                MskPort *output_port = input_port->input.connection;

                // TODO: This is an ugly workaround!
                void *output_buffer = msk_module_get_output_buffer(output_port->owner, output_port->name);
                void *input_buffer = input_port->buffer;

                task->adapter.callback(output_buffer, input_buffer, start, nframes);
            }
            else if ( task->type == MSK_DEFAULTVALUE )
            {
                MskPort *input_port = task->defaultvalue.input_port;

                if ( input_port->port_type == MSK_AUDIO_DATA )
                {
                    float *buffer = input_port->buffer;
                    int i;

                    for ( i = start; i < start + nframes; i++ )
                        buffer[i] = input_port->default_value;
                }
                else if ( input_port->port_type == MSK_CONTROL_DATA )
                {
                    float *buffer = input_port->buffer;
                    *buffer = input_port->default_value;
                }
                else
                    g_error("Unknown port type.");
            }
        }
    }
}


static void msk_container_voices_changed(MskProperty *property, void *buffer)
{
    MskContainer *container = property->owner->container;
    int *value = (int *)buffer;

    if ( *value < 0 )
        *value = 1;

    container->voices = *value;
}

MskContainer *msk_container_create(MskContainer *parent)
{
    MskContainer *container;
    MskModule *module;
    MskProperty *property;

    // Create the shell/wrapper/outside/whatever module.
    module = msk_module_create(parent, "container", NULL);

    container = g_new0(MskContainer, 1);
    container->module = module;
    container->voices = 1;
    module->container = container;

    msk_add_integer_property(module, "type", MSK_SIMPLE_CONTAINER);
    property = msk_add_integer_property(module, "voices", container->voices);
    msk_property_set_write_callback(property, msk_container_voices_changed);
    msk_property_set_flags(property, MSK_PROPERTY_DEACTIVATES_MODULE);

    return container;
}



static gboolean can_split_a_delay(MskContainer *container)
{
    /* Search for the delay with the highest block size limiter.. */

    MskModule *mod, *split_mod = NULL;
    GList *lmod;

    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        mod = lmod->data;

        if ( mod->can_split && !mod->is_split )
            if ( !split_mod || split_mod->delay_limiter < mod->delay_limiter )
                split_mod = mod;
    }

    if ( !split_mod )
        return FALSE;

    split_mod->is_split = TRUE;
    return TRUE;
}

static gboolean check_for_loop(MskModule *module)
{
    GList *lport;

    if ( module->is_split )
        return FALSE;

    if ( module->could_cause_loop == TRUE )
        return TRUE;

    module->could_cause_loop = TRUE;

    for ( lport = module->in_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        MskModule *linked_mod;

        if ( !port->input.connection )
            continue;

        linked_mod = port->input.connection->owner;

        if ( linked_mod->prepared )
            continue;
        else
        {
            if ( check_for_loop(linked_mod) )
                return TRUE;
        }
    }

    module->could_cause_loop = FALSE;
    module->prepared = TRUE;

    return FALSE;
}

static gboolean container_has_loop(MskContainer *container)
{
    MskModule *mod;
    GList *lmod;

    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        mod = lmod->data;
        mod->is_split = FALSE;
    }

    container->block_size_limit = 0;

    while ( TRUE )
    {
        gboolean has_loop = FALSE;

        for ( lmod = container->module_list; lmod; lmod = lmod->next )
        {
            mod = lmod->data;
            mod->prepared = FALSE;
            mod->could_cause_loop = FALSE;
        }

        for ( lmod = container->module_list; lmod; lmod = lmod->next )
        {
            mod = lmod->data;

            if ( !mod->prepared )
            {
                if ( check_for_loop(mod) )
                {
                    has_loop = TRUE;
                    break;
                }
            }
        }

        if ( !has_loop )
            return FALSE;

        if ( !can_split_a_delay(container) )
            return TRUE;

        /* If a split could be made, try again. */
    }
}


static MskProcessor *add_processor(guint type, GList **process_order)
{
    MskProcessor *task = g_new0(MskProcessor, 1);
    task->type = type;

    // TODO: g_list_append takes time... it's better to prepend on linked
    // lists, and then reverse the order.
    *process_order = g_list_append(*process_order, task);

    return task;
}

static void add_module_as_task(MskModule *mod, GList **process_order)
{
    GList *lport;
    MskProcessor *task;

    mod->prepared = TRUE;

    for ( lport = mod->in_ports; lport; lport = lport->next )
    {
        MskPort *port = (MskPort*) lport->data;
        MskPort *linked_port;

        if ( !port->input.connection )
        {
            /* Add a task that fills the port with its default value. */
            task = add_processor(MSK_DEFAULTVALUE, process_order);
            task->defaultvalue.input_port = port;

            continue;
        }

        linked_port = port->input.connection;

        if ( linked_port->owner->is_split )
        {
            if ( !linked_port->owner->split_prepared )
            {
                task = add_processor(MSK_HALFPROCESSOR, process_order);
                task->halfprocessor.module = linked_port->owner;
                task->halfprocessor.input = FALSE;
                linked_port->owner->split_prepared = TRUE;
            }
        }
        else if ( !linked_port->owner->prepared )
            add_module_as_task(linked_port->owner, process_order);

        /* If it needs an adapter for this port, add it as a task. */
        if ( port->port_type != linked_port->port_type )
        {
            task = add_processor(MSK_ADAPTER, process_order);
            task->adapter.callback = msk_get_adapter(linked_port->port_type,
                    port->port_type);
            task->adapter.input_port = port;
            g_assert(task->adapter.callback != NULL);
        }
    }

    if ( !mod->is_split )
    {
        task = add_processor(MSK_PROCESSOR, process_order);
        task->processor.module = mod;
    }
    else
    {
        task = add_processor(MSK_HALFPROCESSOR, process_order);
        task->halfprocessor.module = mod;
        task->halfprocessor.input = TRUE;
    }
}


static void print_list_order(GList *items, gint indent)
{
    GList *item;

    for ( item = items; item; item = item->next )
    {
        MskProcessor *task = item->data;

        if ( task->type == MSK_PROCESSOR )
        {
            MskModule *mod = task->processor.module;

            g_print(" %*s- %s\n", indent, "", mod->name);
            if ( mod->container )
                print_list_order(mod->container->processing_list, indent + 2);
        }
        else if ( task->type == MSK_HALFPROCESSOR )
        {
            MskModule *mod = task->halfprocessor.module;

            g_print(" %*s- %s (%s-half)\n", indent, "", mod->name,
                    task->halfprocessor.input ? "input" : "output");
        }
        else if ( task->type == MSK_ADAPTER )
            g_print(" %*s- [port adapter]\n", indent, "");
    }
}


/* Attempt to do a topological sort of its modules, and construct a list of
 * 'tasks' from that. */
gboolean msk_container_sort(MskContainer *container)
{
    GList *processing_list = NULL;
    GList *lmod;

    if ( container_has_loop(container) )
        g_error("Loop in container.");

    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        ((MskModule *)lmod->data)->prepared = FALSE;
        ((MskModule *)lmod->data)->split_prepared = FALSE;
    }

    for ( lmod = container->module_list; lmod; lmod = lmod->next )
    {
        MskModule *mod = lmod->data;

        if ( !mod->prepared )
            add_module_as_task(mod, &processing_list);

        if ( mod->is_split && mod->delay_limiter &&
                (mod->delay_limiter < container->block_size_limit ||
                 !container->block_size_limit ) )
            container->block_size_limit = mod->delay_limiter;
    }

    if ( container->processing_list )
    {
        GList *item;

        for ( item = container->processing_list; item; item = item->next )
            g_free(item->data);
        g_list_free(container->processing_list);
    }

    print_list_order(processing_list, 0);
    g_print("Container block size limit: %d.\n", container->block_size_limit);

    container->processing_list = processing_list;
    return TRUE;
}


static void msk_input_destroy(MskModule *self)
{
    MskContainer *parent = self->parent;

    parent->input_modules = g_list_remove(parent->input_modules, self);
}

MskModule *msk_input_create_with_name(MskContainer *parent, gchar *name, guint type)
{
    MskModule *mod;
    MskPort *interior_port;
    MskPort *exterior_port;

    mod = msk_module_create(parent, "input", NULL);

    exterior_port = msk_add_input_port(parent->module, name, type, 0.0f);  // ?
    interior_port = msk_add_output_port(mod, exterior_port->name, type);

    parent->input_modules = g_list_append(parent->input_modules, mod);
    msk_add_destroy_callback(mod, msk_input_destroy);

    msk_meld_ports(exterior_port, interior_port);

    return mod;
}

MskModule *msk_input_create(MskContainer *parent)
{
    return msk_input_create_with_name(parent, "in#", MSK_AUDIO_DATA);
}

void msk_output_process(MskModule *self, int start, int frames, void *state)
{
    // TODO: FIX THIIIS!
    // We need a better way to handle ports of unknown name.
    // Also, we need to get rid of this 'mix_to' completely.
    char *name = ((MskPort*)self->in_ports->data)->name;
    const float * const in = msk_module_get_input_buffer(self, name);
    float * const mix_to = self->mix_to->buffer;
    int i;

    for ( i = start; i < start + frames; i++ )
        mix_to[i] += in[i];
}

MskModule *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type)
{
    MskModule *mod;
    MskPort *interior_port;
    MskPort *exterior_port;

    mod = msk_module_create(parent, "output", msk_output_process);

    exterior_port = msk_add_output_port(parent->module, name, type);
    interior_port = msk_add_input_port(mod, exterior_port->name, type, 0.0f);  // ?

    mod->mix_to = exterior_port;

    // This only works on monophonic containers..
    //msk_meld_ports(interior_port, exterior_port);

    return mod;
}

MskModule *msk_output_create(MskContainer *parent)
{
    return msk_output_create_with_name(parent, "out#", MSK_AUDIO_DATA);
}


void msk_voicenumber_process(MskModule *self, int start, int frames, void *state)
{
    float * const out = msk_module_get_output_buffer(self, "nr");
    const int voice = self->parent->current_voice;

    *out = voice;
}

/* Output the number of the current voice. */
MskModule *msk_voicenumber_create(MskContainer *parent)
{
    MskModule *mod;

    mod = msk_module_create(parent, "voicenumber", msk_voicenumber_process);

    msk_add_output_port(mod, "nr", MSK_CONTROL_DATA);

    return mod;
}


void msk_instrument_voices_changed(MskProperty *property, void *buffer)
{
    MskContainer *container = property->owner->container;
    MskInstrument *instrument = container->instrument;
    int *value = (int *)buffer;

    if ( *value < 0 )
        *value = 1;

    container->voices = *value;
    g_free(instrument->voice_active);
    g_free(instrument->voice_note);
    g_free(instrument->voice_velocity);
    instrument->voice_active = g_new0(char, container->voices);
    instrument->voice_note = g_new0(short, container->voices);
    instrument->voice_velocity = g_new0(short, container->voices);
}


void msk_instrument_channel_changed(MskProperty *property, void *buffer)
{
    MskInstrument *instrument = property->owner->container->instrument;
    int *value = (int *)buffer;

    if ( *value < 0 || *value > 16 )
        *value = 0;

    instrument->channel = *value;
}


static void msk_instrument_destroy(MskModule *self)
{
    MskWorld *world = self->world;
    MskInstrument *instrument = self->container->instrument;

    world->instruments = g_list_remove(world->instruments, instrument);

    // TODO: destroy it for good.
}

// This is almost identical with msk_container_create... and that's a
// problem.
MskContainer *msk_instrument_create(MskContainer *parent)
{
    MskWorld *world;
    MskContainer *container;
    MskInstrument *instrument;
    MskModule *module;
    MskProperty *property;

    // Create the shell/wrapper/outside/whatever module.
    module = msk_module_create(parent, "instrument", NULL);

    container = g_new0(MskContainer, 1);
    container->module = module;
    container->voices = 8;
    module->container = container;

    instrument = g_new0(MskInstrument, 1);
    instrument->container = container;
    instrument->voice_active = g_new0(char, container->voices);
    instrument->voice_note = g_new0(short, container->voices);
    instrument->voice_velocity = g_new0(short, container->voices);
    container->instrument = instrument;

    // Add to the world
    world = module->world;
    world->instruments = g_list_prepend(world->instruments, instrument);

    msk_add_integer_property(module, "type", MSK_INSTRUMENT_CONTAINER);
    property = msk_add_integer_property(module, "voices", container->voices);
    msk_property_set_write_callback(property, msk_instrument_voices_changed);
    msk_property_set_flags(property, MSK_PROPERTY_DEACTIVATES_MODULE);
    property = msk_add_integer_property(module, "channel", instrument->channel);
    msk_property_set_write_callback(property, msk_instrument_channel_changed);

    msk_add_destroy_callback(module, msk_instrument_destroy);

    return container;
}
