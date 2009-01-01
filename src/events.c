#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


/* TODO:
 *  - figure out mouse buttons
 */


extern void virkb_keypress(int key);
extern void virkb_keyrelease(int key);
extern void virkb_mouseon(gdouble x, gdouble y);
extern void virkb_mouseoff();
extern void emulate_control_change(int channel, int control, int value);

extern GtkWidget *properties_frame;

static const guint compkey_notes[] =
{
    GDK_z, GDK_s, GDK_x, GDK_d, GDK_c, GDK_v, GDK_g, GDK_b, GDK_h, GDK_n, GDK_j, GDK_m,
    GDK_q, GDK_2, GDK_w, GDK_3, GDK_e, GDK_r, GDK_5, GDK_t, GDK_6, GDK_y, GDK_7, GDK_u,
    GDK_i, GDK_9, GDK_o, GDK_0, GDK_p, GDK_bracketleft, GDK_equal, GDK_bracketright,
    0
};

static const guint alternate_compkey_notes[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    GDK_comma, GDK_l, GDK_period, GDK_semicolon, GDK_slash,
    0
};


G_MODULE_EXPORT void on_drawingarea3_button_press_event(GtkObject *object,
                                                        GdkEventButton *event)
{
    virkb_mouseon(event->x, event->y);
    
    g_print("Button %d pressed.\n", event->button);
    
    gtk_widget_grab_focus(GTK_WIDGET(object));
}

G_MODULE_EXPORT void on_drawingarea3_button_release_event(GtkObject *object,
                                                        GdkEventExpose *event)
{
    virkb_mouseoff();
}

G_MODULE_EXPORT gboolean on_drawingarea3_key_event(GtkObject *object,
                                                   GdkEventKey *event)
{
    guint key, i;
    
    key = gdk_keyval_to_lower(event->keyval);
    
    for ( i = 0; compkey_notes[i]; i++ )
    {
        if ( compkey_notes[i] == key ||
             alternate_compkey_notes[i] == key )
        {
            if ( event->type == GDK_KEY_PRESS )
                virkb_keypress(i + 60);
            else if ( event->type == GDK_KEY_RELEASE )
                virkb_keyrelease(i + 60);
            
            return TRUE;
        }
    }
    
    /* Propagate event further. */
    return FALSE;
}

G_MODULE_EXPORT void on_drawingarea3_motion_notify_event(GtkObject *object,
                                                         GdkEventMotion *event)
{
    if ( event->state & GDK_BUTTON1_MASK )
        virkb_mouseon(event->x, event->y);
}

G_MODULE_EXPORT void on_window_focus_out_event(GtkObject *object,
                                               GdkEventExpose *event)
{
    virkb_mouseoff();
    g_print("Focus out.\n");
}

G_MODULE_EXPORT void on_vscale_mw_value_changed(GtkRange *range)
{
    emulate_control_change(1, 1, (int) gtk_range_get_value(range));
}

G_MODULE_EXPORT void on_vscale_pw_value_changed(GtkObject *object)
{
    
}

G_MODULE_EXPORT void on_menuitem_properties_activate(GtkMenuItem *menuitem,
                                                     gpointer     user_data)
{
    int visible;
    
    /* A show/hide toggle for the Properties frame. */
    g_object_get(G_OBJECT(properties_frame), "visible", &visible, NULL);
    visible = !visible;
    g_object_set(G_OBJECT(properties_frame), "visible", visible, NULL);
}
