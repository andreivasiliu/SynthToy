#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>

#include "msk0/msk0.h"
#include "gmsk/gmsk.h"
#include "header.h"


/* TODO:
 *  - figure out mouse buttons
 */


extern void virkb_keypress(int key);
extern void virkb_keyrelease(int key);
extern void virkb_mouseon(gdouble x, gdouble y);
extern void virkb_mouseoff();
extern void emulate_control_change(int channel, int control, int value);

extern float array[512];

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

static const struct
{
    guint key;
    char *modulename;
} module_shortcuts[] =
{
    { GDK_A, "add" },
    { GDK_M, "mul" },
    { GDK_C, "constant" },
    { GDK_I, "input" },
    { GDK_O, "output" },
    { GDK_D, "delay" },
    { 0, NULL }
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

G_MODULE_EXPORT gboolean on_drawingarea2_key_event(GtkObject *object,
                                                   GdkEventKey *event)
{
    if ( event->keyval == GDK_Delete && event->type == GDK_KEY_PRESS )
    {
        gmsk_delete_selected();
        return TRUE;
    }

    if ( event->type == GDK_KEY_PRESS )
    {
        int i;

        for ( i = 0; module_shortcuts[i].modulename; i++ )
            if ( event->keyval == module_shortcuts[i].key )
            {
                gmsk_create_module(module_shortcuts[i].modulename);
                return TRUE;
            }
    }

    on_drawingarea3_key_event(GTK_OBJECT(virkb), event);

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

G_MODULE_EXPORT void on_drawingarea1_expose_event(GtkObject *object,
        GdkEventExpose *event)
{
    paint_array_to_widget(GTK_WIDGET(object), array, 512);
}

G_MODULE_EXPORT void on_drawingarea2_expose_event(GtkObject *object,
        GdkEventExpose *event)
{
    cairo_t *cr;

    cr = gdk_cairo_create(GTK_WIDGET(object)->window);

    gmsk_paint_editor(cr);
}

G_MODULE_EXPORT void on_drawingarea3_expose_event(GtkObject *object,
        GdkEventExpose *event)
{
    paint_keyboard(GTK_WIDGET(object));
}

G_MODULE_EXPORT gboolean
    on_drawingarea2_motion_notify_event(GtkObject *object,
                                        GdkEventMotion *event)
{
    return gmsk_mouse_motion_event(event->x, event->y, event->state);
}

G_MODULE_EXPORT gboolean
    on_drawingarea2_button_press_event(GtkObject *object,
                                       GdkEventButton *event)
{
    gtk_widget_grab_focus(GTK_WIDGET(object));

    if ( event->button == 3 )
    {
        GtkWidget *menu;

        menu = create_menu();

        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                       event->button, event->time);

        return TRUE;
    }

    return gmsk_mouse_press_event(event->x, event->y, event->button, event->type, event->state);
}

G_MODULE_EXPORT gboolean
    on_drawingarea2_button_release_event(GtkObject *object,
                                         GdkEventButton *event)
{
    return gmsk_mouse_release_event(event->x, event->y, event->button, event->state);
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


G_MODULE_EXPORT void on_menuitem_quit_activate(GtkMenuItem *menuitem,
                                               gpointer     user_data)
{
    // TODO: maybe ask if the file should be saved?

    gtk_widget_destroy(main_window);
}


G_MODULE_EXPORT void on_menuitem_open_activate(GtkMenuItem *menuitem,
                                               gpointer     user_data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(main_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, ".msk files");
    gtk_file_filter_add_pattern(filter, "*.msk");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        MskContainer *new_world;
        char *filename;
        GError *error = NULL;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        new_world = gmsk_load_world_from_file(filename, &error);

        if ( error )
        {
            GtkWidget *dialog;

            dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                    GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error->message);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
        else
        {
            gmsk_lock_mutex();
            msk_world_destroy(aural_root);
            aural_root = new_world;
            gmsk_unlock_mutex();
        }
    }

    gtk_widget_destroy(dialog);
}


G_MODULE_EXPORT void on_menuitem_save_activate(GtkMenuItem *menuitem,
                                               gpointer     user_data)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new("Save File",
                                         GTK_WINDOW(main_window),
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);

//    if (user_edited_a_new_document)
//    {
//        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), default_folder_for_saving);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), "Untitled");
//    }
//    else
//        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), filename_for_existing_document);

    if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GError *error = NULL;

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
        gmsk_save_world_to_file(filename, &error);
        g_free (filename);

        if ( error )
        {
            GtkWidget *dialog;

            dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                    GTK_DIALOG_DESTROY_WITH_PARENT,
                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error->message);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }

    gtk_widget_destroy (dialog);



    // TODO: this is just for testing.
}


G_MODULE_EXPORT void on_propertycell_edited(GtkCellRendererText *renderer,
                                            gchar               *path,
                                            gchar               *new_text,
                                            gpointer             user_data)
{
    GtkTreeIter iter;
    char *property_name;
    char *newer_text;
    MskModule *module;
    MskProperty *property;

    module = gmsk_get_selected_module();
    if ( !module )
        return;

    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(liststore_properties), &iter, path);
    gtk_tree_model_get(GTK_TREE_MODEL(liststore_properties), &iter, 0, &property_name, -1);

    gmsk_lock_mutex();
    property = msk_module_get_property(module, property_name);
    msk_property_set_value_from_string(property, new_text);
    newer_text = msk_property_get_value_as_string(property);
    gmsk_unlock_mutex();

    gtk_list_store_set(liststore_properties, &iter, 1, newer_text, -1);
    g_free(newer_text);
}

void on_module_selected(MskModule *module, void *userdata)
{
    GList *item;

    gtk_list_store_clear(liststore_properties);

    if ( !module )
        return;

    /* Populate the Properties list with this module's properties. */
    for ( item = module->properties; item; item = item->next )
    {
        MskProperty *property = item->data;
        const void *value = property->value;
        char *string_value;
        GtkTreeIter iter;

        if ( property->type == MSK_INT_PROPERTY )
            string_value = g_strdup_printf("%d", *(int *) value);
        else if ( property->type == MSK_FLOAT_PROPERTY )
            string_value = g_strdup_printf("%f", *(float *) value);
        else if ( property->type == MSK_STRING_PROPERTY )
            string_value = g_strdup((char *) value);
        else
            g_error("Unkown property type.");

        gtk_list_store_append(liststore_properties, &iter);
        gtk_list_store_set(liststore_properties, &iter,
                0, property->name, 1, string_value, -1);

        g_free(string_value);
    }
}

void on_editor_invalidated(void *userdata)
{
    gtk_widget_queue_draw(GTK_WIDGET(editor));
}


