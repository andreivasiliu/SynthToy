
// Emptyish... for now.

/* mskmodule.c */

extern MskModule *msk_module_create(MskContainer *parent, gchar *name,
                                    MskProcessCallback process,
                                    MskActivateCallback activate,
                                    MskDeactivateCallback deactivate,
                                    gsize state_size);
MskPort *msk_add_input_port(MskModule *mod, gchar *name, guint type, gfloat default_value);
MskPort *msk_add_output_port(MskModule *mod, gchar *name, guint type);
MskProperty *msk_add_float_property(MskModule *mod, gchar *name, gfloat value);
MskProperty *msk_add_integer_property(MskModule *mod, gchar *name, gint value);

void msk_meld_ports(MskPort *port1, MskPort *port2);
