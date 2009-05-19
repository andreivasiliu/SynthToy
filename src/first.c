#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <stdlib.h>  // for exit()

#include "header.h"

#ifdef HAVE_JACK
# include "mod-jack/modjack.h"
#else
# include "mod-winmm/modwinmm.h"
#endif

extern void paint_keyboard(GtkWidget *widget);

GtkWidget *main_window;
GtkWidget *left_osc, *editor, *virkb, *vscale_pw, *vscale_mw;
GtkWidget *properties_frame;
#ifdef HAVE_JACK
void *jack_instance;
#else
void *winmm_instance;
#endif
float array[512];
float array2[512];

int screenupdates;
extern gdouble processing_time;

guint timeout;
guint pt_timeout;


G_MODULE_EXPORT void on_window_destroy(GtkObject *object, gpointer user_data)
{
#ifdef HAVE_JACK
    modjack_deactivate(jack_instance);
    modjack_fini(jack_instance);
#else
    modwinmm_deactivate(winmm_instance);
    modwinmm_fini(winmm_instance);
#endif
    
    g_source_remove(timeout);
    g_source_remove(pt_timeout);
    
    gtk_main_quit();
}


void paint_array_to_widget(GtkWidget *widget, float *array, int length)
{
    cairo_t *cr;
    int i, width, height;
    
    gdk_window_get_size(widget->window, &width, &height);
    
    cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
   
    cairo_save(cr);
    cairo_scale(cr, (float) width / (float) length, (float) height / 100.0f);
    
    cairo_move_to(cr, 0, 50 + (array[0] * 25) - 0.5);
    
    for ( i = 1; i < length; i++ )
    {
        cairo_line_to(cr, i, 50 + (array[i] * 25) - 0.5);
    }
    
    cairo_restore(cr);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_set_line_width(cr, 1);
    cairo_stroke(cr);
    
    cairo_destroy(cr);
}


G_MODULE_EXPORT void on_drawingarea1_expose_event(GtkObject *object, GdkEventExpose *event)
{
    paint_array_to_widget(GTK_WIDGET(object), array, 512);
}

G_MODULE_EXPORT void on_drawingarea3_expose_event(GtkObject *object, GdkEventExpose *event)
{
    paint_keyboard(GTK_WIDGET(object));
}


gboolean periodic_refresh(gpointer instance)
{
    gtk_widget_queue_draw(GTK_WIDGET(left_osc));
    
    return TRUE;
}

gboolean pt_update(gpointer instance)
{
    g_print("CPU usage: %d%%.\n", (int)(processing_time*100));
    processing_time = 0;
    
    return TRUE;
}


int main(int argc, char *argv[])
{
    GtkBuilder	*builder;
    char	*errmsg;
    
    g_thread_init(NULL);
//    gdk_threads_init();
    
#ifdef HAVE_JACK
    jack_instance = modjack_init(process_func, event_func, NULL, &errmsg);
    if ( !jack_instance )
    {
        g_error("Couldn't initialize the jack client (%s).", errmsg);
        return 1;
    }
#else
    winmm_instance = modwinmm_init(process_func, event_func, NULL, &errmsg);
    if ( !winmm_instance )
    {
        g_error("Couldn't initialize the winmm client (%s).", errmsg);
        return 1;
    }
#endif    
    
#ifdef WIN32
    gtk_init(NULL, NULL);
#else
    gtk_init(&argc, &argv);
#endif
    
    builder = gtk_builder_new();
    if ( !gtk_builder_add_from_file(builder, "first.ui", NULL) )
        g_error("I can't find my 'first.ui' file.");
    
    main_window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    left_osc = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea1"));
    editor = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea2"));
    virkb = GTK_WIDGET(gtk_builder_get_object(builder, "drawingarea3"));
    vscale_pw = GTK_WIDGET(gtk_builder_get_object(builder, "vscale_pw"));
    vscale_mw = GTK_WIDGET(gtk_builder_get_object(builder, "vscale_mw"));
    properties_frame = GTK_WIDGET(gtk_builder_get_object(builder, "properties_frame"));
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(G_OBJECT(builder));
    
    gtk_widget_show(main_window);
    
    timeout = g_timeout_add(50, periodic_refresh, NULL);
    pt_timeout = g_timeout_add(1000, pt_update, NULL);
    
    aural_init();
   
#ifdef HAVE_JACK
    modjack_activate(jack_instance);
#else
    modwinmm_activate(winmm_instance);
#endif
    
//    gdk_threads_enter();
    gtk_main();
//    gdk_threads_leave();
    
    // This is here for the Windows version, which hangs withou it.
    exit(0);

    return 0;
}
