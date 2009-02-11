
// Emptyish... for now.

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
