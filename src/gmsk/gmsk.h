
// These coincide with the constants from GDK.
enum
{
    GMSK_SHIFT_MASK     = (1 << 0),
    GMSK_CONTROL_MASK   = (1 << 2),
    GMSK_2BUTTON_PRESS  = 5,
};


/* gmskmain.c */
typedef void (*GmskInvalidateCallback)(gpointer user_data);
typedef void (*GmskSelectModuleCallback)(MskModule *module, gpointer user_data);

void gmsk_init(MskContainer *root);
void gmsk_draw_module_at(MskModule *module, int x, int y);
void gmsk_set_invalidate_callback(GmskInvalidateCallback callback, gpointer user_data);
void gmsk_set_select_module_callback(GmskSelectModuleCallback callback, gpointer user_data);
void gmsk_lock_mutex();
void gmsk_unlock_mutex();
MskModule *gmsk_create_module(char *name);
MskContainer *gmsk_create_container(char *name);


/* gmsksaveload.c */
gboolean gmsk_save_world_to_file(const gchar *filename, GError **error);
gboolean gmsk_load_world_from_file(const gchar *filename, GError **error);


/* gmskeditor.c */
void gmsk_paint_editor(cairo_t *cr);
gboolean gmsk_mouse_motion_event(int x, int y, int modifiers);
gboolean gmsk_mouse_press_event(int x, int y, int button, int type, int modifiers);
gboolean gmsk_mouse_release_event(int x, int y, int button, int modifiers);
MskModule *gmsk_get_selected_module();


/* gmskmacros.c */
void gmsk_create_macro(char *name);

