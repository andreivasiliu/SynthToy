
#ifdef WIN32
 #ifdef LIBGMSK_EXPORTS
  #define GMSK_API __declspec(dllexport)
 #else
  #define GMSK_API __declspec(dllimport)
 #endif
#else
# define GMSK_API
#endif


// These should coincide with the constants from GDK.
enum
{
    GMSK_SHIFT_MASK     = (1 << 0),
    GMSK_CONTROL_MASK   = (1 << 2),
    GMSK_2BUTTON_PRESS  = 5,
};


/* gmskmain.c */
typedef void (*GmskInvalidateCallback)(gpointer user_data);
typedef void (*GmskSelectModuleCallback)(MskModule *module, gpointer user_data);

void GMSK_API gmsk_init(MskContainer *root);
void GMSK_API gmsk_draw_module_at(MskModule *module, int x, int y);
void GMSK_API gmsk_set_invalidate_callback(GmskInvalidateCallback callback, gpointer user_data);
void GMSK_API gmsk_set_select_module_callback(GmskSelectModuleCallback callback, gpointer user_data);
void GMSK_API gmsk_lock_mutex();
void GMSK_API gmsk_unlock_mutex();
MskModule GMSK_API *gmsk_create_module(char *name);
MskContainer GMSK_API *gmsk_create_container(char *name);


/* gmsksaveload.c */
gboolean GMSK_API gmsk_save_world_to_file(const gchar *filename, GError **error);
gboolean GMSK_API gmsk_load_world_from_file(const gchar *filename, GError **error);


/* gmskeditor.c */
void GMSK_API gmsk_paint_editor(cairo_t *cr);
gboolean GMSK_API gmsk_mouse_motion_event(int x, int y, int modifiers);
gboolean GMSK_API gmsk_mouse_press_event(int x, int y, int button, int type, int modifiers);
gboolean GMSK_API gmsk_mouse_release_event(int x, int y, int button, int modifiers);
MskModule GMSK_API *gmsk_get_selected_module();
MskPort GMSK_API *msk_get_selected_connection();
void GMSK_API gmsk_delete_selected();



/* gmskmacros.c */
void GMSK_API gmsk_create_macro(char *name);

