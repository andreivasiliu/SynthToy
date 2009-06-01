
typedef void (*MskAdapterCallback)(void *source, void *destination, int start, int frames);

typedef enum
{
    MSK_PROCESSOR,
    MSK_HALFPROCESSOR,
    MSK_ADAPTER,
    MSK_DEFAULTVALUE,
} MskProcessorType;

/* This is used as a 'task' in a list of processing items. */
struct _MskProcessor
{
    MskProcessorType type;

    union
    {
        /* Type 1.. calls a module's 'process' callback. */
        struct
        {
            MskModule *module;
        } processor;

        /* Type 2.. calls a module's 'process_input' or 'process_output'
         * callback. */
        struct
        {
            MskModule *module;
            gboolean input;
        } halfprocessor;

        /* Type 3.. converts data between different port types. */
        struct
        {
            MskAdapterCallback callback;
            MskPort *input_port;
        } adapter;

        /* Type 4.. supplies an input port with its default value. */
        struct
        {
            MskPort *input_port;
        } defaultvalue;
    };
};


/* mskadapter.c */

MskAdapterCallback msk_get_adapter(guint source_port_type, guint destination_port_type);


/* mskmodule.c */

extern MskModule *msk_module_create(MskContainer *parent, gchar *name,
                                    MskProcessCallback process);

void msk_add_state(MskModule *module, MskActivateCallback activate,
                   MskDeactivateCallback deactivate, gsize state_size);

void *msk_add_global_state(MskModule *module, gsize state_size);

void msk_add_destroy_callback(MskModule *module, MskModuleDestroyCallback callback);

void msk_add_split_personality(MskModule *module,
                               MskProcessCallback process_input,
                               MskProcessCallback process_output);

void msk_dynamic_ports(MskModule *module, gint dynamic_group_size,
                       MskDynamicPortAddCallback dynamic_port_add,
                       MskDynamicPortRemoveCallback dynamic_port_remove);

void msk_add_port_to_buffer_group(MskPort *port, int group);

void msk_module_activate(MskModule *mod);

gpointer msk_port_get_input_buffer(MskPort *input_port);
gpointer msk_port_get_output_buffer(MskPort *output_port);
gpointer msk_port_get_buffer(MskPort *port);

MskPort *msk_add_input_port(MskModule *mod, gchar *name, guint type, gfloat default_value);
void msk_remove_input_port(MskModule *mod);
MskPort *msk_add_output_port(MskModule *mod, gchar *name, guint type);
MskProperty *msk_add_float_property(MskModule *mod, gchar *name, gfloat value);
MskProperty *msk_add_integer_property(MskModule *mod, gchar *name, gint value);
MskProperty *msk_add_string_property(MskModule *mod, gchar *name, gchar *value);
void msk_property_set_write_callback(MskProperty *property, MskPropertyWriteCallback callback);
void msk_property_set_flags(MskProperty *property, guint flags);

void msk_container_activate(MskContainer *self);
void msk_container_process(MskContainer *self, int start, int nframes, guint voice);
gboolean msk_container_sort(MskContainer *container);
void msk_meld_ports(MskPort *port1, MskPort *port2);
