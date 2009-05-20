
/* mskadapter.c */

typedef void (*MskAdapterCallback)(void *source, void *destination, int start, int frames);

typedef enum
{
    MSK_PROCESSOR,
    MSK_ADAPTER,
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

        /* Type 2.. converts data between different port types */
        struct
        {
            MskAdapterCallback callback;
            MskPort *input_port;
        } adapter;
    };
};

MskAdapterCallback msk_get_adapter(guint source_port_type, guint destination_port_type);


/* mskmodule.c */

extern MskModule *msk_module_create(MskContainer *parent, gchar *name,
                                    MskProcessCallback process);

void msk_add_state(MskModule *module, MskActivateCallback activate,
                   MskDeactivateCallback deactivate, gsize state_size);

void msk_add_global_state(MskModule *module,
                          MskGlobalActivateCallback global_activate,
                          MskGlobalActivateCallback global_deactivate,
                          gsize state_size);

void msk_dynamic_ports(MskModule *module,
                       MskDynamicPortAddCallback dynamic_port_add,
                       MskDynamicPortRemoveCallback dynamic_port_remove);

void msk_module_activate(MskModule *mod);

MskPort *msk_add_input_port(MskModule *mod, gchar *name, guint type, gfloat default_value);
void msk_remove_input_port(MskModule *mod);
MskPort *msk_add_output_port(MskModule *mod, gchar *name, guint type);
MskProperty *msk_add_float_property(MskModule *mod, gchar *name, gfloat value);
MskProperty *msk_add_integer_property(MskModule *mod, gchar *name, gint value);

void msk_container_activate(MskContainer *self);
void msk_container_process(MskContainer *self, int start, int nframes, guint voice);
gboolean msk_container_sort(MskContainer *container);
void msk_meld_ports(MskPort *port1, MskPort *port2);
