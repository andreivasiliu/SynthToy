#include <gtk/gtk.h>
#include <cairo.h>
#define PANGO_ENABLE_BACKEND 1
#include <pango/pango.h>
#include <pango/pangocairo.h>
#undef PANGO_ENABLE_BACKEND

#include <libintl.h>

#include "msk0/msk0.h"


/* WARNING: This code might be abusing a bug in Pango. */

cairo_surface_t *surface;

typedef struct _GraphicalModule GraphicalModule;

struct _GraphicalModule
{
    cairo_surface_t *surface;
    MskModule *mod;
    
    long x, y;
    
    long width;
    long height;
    
    int dragged;
    int drag_x;
    int drag_y;
};


GList *graphical_modules;

GraphicalModule *dragged_module;
int drag_grip_x;
int drag_grip_y;

cairo_surface_t *create_my_thing()
{
    int width = 60, height = 45;
    int l_width, l_height;
    
    
    cairo_surface_t *surface;
    cairo_t *cr;
    PangoLayout *p_layout;
    PangoFontDescription *p_font_desc;
    
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    
    cr = cairo_create(surface);
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    
    cairo_set_line_width(cr, 1);
    
    cairo_rectangle(cr, 0.5, 0.5, width - 1, height - 1);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_stroke(cr);
    
    cairo_rectangle(cr, 1.5, 1.5, width - 3, height - 3);
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_stroke(cr);
    
    cairo_rectangle(cr, 2.5, 2.5, width - 5, height - 5);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_stroke(cr);
    
    p_layout = pango_cairo_create_layout(cr);
    p_font_desc = pango_font_description_from_string("Sans 8");
    pango_layout_set_font_description(p_layout, p_font_desc);
    pango_font_description_free(p_font_desc);
    /* TRANSLATORS: This will be shown inside a small box, as a test.
     * Nothing more to it. */
    pango_layout_set_text(p_layout, gettext("Hello!"), -1);
    
    pango_layout_get_pixel_size(p_layout, &l_width, &l_height);
    g_print("%d/%d\n", l_width, l_height);
    
    cairo_move_to(cr, width / 2 - l_width / 2, height / 2 - l_height / 2);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    pango_cairo_show_layout(cr, p_layout);
    
    g_object_unref(p_layout);
    cairo_destroy(cr);
    
    return surface;
}


void draw_module(MskModule *mod, long x, long y)
{
    GraphicalModule *gmod;
    PangoContext *context;
    PangoFontMap *font_map;
    PangoLayout *title;
    PangoFontDescription *font_desc;
    cairo_surface_t *surface;
    cairo_t *cr;
    int height, width;
    int surface_height, surface_width;
    
    gmod = g_new0(GraphicalModule, 1);
    
    /* Before we start drawing, we need to determine the minimum size of the
     * surface. */
    
    /* First, the border. */
    surface_width = 6;
    surface_height = 6;
    
    /* The title/name. */
    
    font_map = pango_cairo_font_map_get_default();
    context = pango_context_new();
    pango_context_set_font_map(context, font_map);
    
    title = pango_layout_new(context);
    font_desc = pango_font_description_from_string("Sans 8");
    pango_layout_set_font_description(title, font_desc);
    pango_font_description_free(font_desc);
    
    pango_layout_set_text(title, mod->name, -1);
    
    pango_layout_get_pixel_size(title, &width, &height);
    
    surface_width += width+1 + 5*2; /* 5 == padding */
    surface_height += height;
    
    /* Separator line */
    surface_height += 3;
    
    /* Let's say, space for some ports */
    surface_height += height*2;
    
    gmod->width = surface_width;
    gmod->height = surface_height;
    
    /* Now that we have the required size, create a surface and start drawing
     * on it. */
    
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         surface_width, surface_height);
    
    cr = cairo_create(surface);
    
    /* Paint the background */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    
    /* Paint the border */
    cairo_set_line_width(cr, 1);
    
    cairo_rectangle(cr, 0.5, 0.5, surface_width - 1, surface_height - 1);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_stroke(cr);
    
    cairo_rectangle(cr, 1.5, 1.5, surface_width - 3, surface_height - 3);
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_stroke(cr);
    
    cairo_rectangle(cr, 2.5, 2.5, surface_width - 5, surface_height - 5);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_stroke(cr);
    
    /* Paint the title */
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    
    pango_layout_get_pixel_size(title, &width, &height);
    cairo_move_to(cr, (surface_width - width) / 2, 4);
    pango_cairo_show_layout(cr, title);
    
    /* Paint the separator */
    
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_move_to(cr, 3, 4.5 + height);
    cairo_line_to(cr, surface_width - 3, 4.5 + height);
    cairo_stroke(cr);
    
    cairo_destroy(cr);
    
    gmod->surface = surface;
    gmod->mod = mod;
    gmod->x = x;
    gmod->y = y;
    
    graphical_modules = g_list_append(graphical_modules, gmod);
}


void paint_editor(GtkWidget *widget)
{
    GList *item;
    cairo_t *cr;
    
//    if ( !surface )
//        surface = create_my_thing();
    
    cr = gdk_cairo_create(widget->window);
    
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.3);
    cairo_paint(cr);
    
    for ( item = graphical_modules; item; item = item->next )
    {
        GraphicalModule *gmod = item->data;
        
        cairo_set_source_surface(cr, gmod->surface, gmod->x, gmod->y);
        
        if ( gmod == dragged_module )
            cairo_paint_with_alpha(cr, 0.8);
        else
            cairo_paint(cr);
    }
    
    cairo_destroy(cr);
}


G_MODULE_EXPORT void
    on_drawingarea2_expose_event(GtkObject *object,
                                                  GdkEventExpose *event)
{
    paint_editor(GTK_WIDGET(object));
}


G_MODULE_EXPORT gboolean
    on_drawingarea2_motion_notify_event(GtkObject *object,
                                        GdkEventMotion *event)
{
    if ( dragged_module )
    {
        /* I wrote this due to lack of inspiration... How else do I round it? */
        dragged_module->x = event->x - drag_grip_x;
        dragged_module->y = event->y - drag_grip_y;
        
        /* Unless 'shift' is pressed, snap to a 5x5 grid. */
        if ( !(event->state & GDK_SHIFT_MASK) )
        {
            dragged_module->x = ((dragged_module->x + 2) -
                                 (dragged_module->x + 2) % 5);
            dragged_module->y = ((dragged_module->y + 2) -
                                 (dragged_module->y + 2) % 5);
        }
        
        if ( dragged_module->x < 0 )
            dragged_module->x = 0;
        if ( dragged_module->y < 0 )
            dragged_module->y = 0;
        
        gtk_widget_queue_draw(GTK_WIDGET(object));
        
        return TRUE;
    }
    
    return FALSE;
}


G_MODULE_EXPORT gboolean
    on_drawingarea2_button_press_event(GtkObject *object,
                                       GdkEventButton *event)
{
    GraphicalModule *gmod;
    GList *item;
    
    /* Only left mouse button can drag. */
    if ( event->button != 1 )
        return FALSE;
    
    /* The last one painted is the first one reachable. */
    item = g_list_last(graphical_modules);
    while ( item )
    {
        gmod = item->data;
        
        if ( event->x >= gmod->x && event->x < gmod->x + gmod->width &&
             event->y >= gmod->y && event->y < gmod->y + gmod->height )
        {
            dragged_module = gmod;
            drag_grip_x = event->x - gmod->x;
            drag_grip_y = event->y - gmod->y;
            
            graphical_modules = g_list_remove(graphical_modules, gmod);
            graphical_modules = g_list_append(graphical_modules, gmod);
            
            gtk_widget_queue_draw(GTK_WIDGET(object));
            
            return TRUE;
        }
        
        item = item->prev;
    }
    
    return FALSE;
}

G_MODULE_EXPORT void on_drawingarea2_button_release_event(GtkObject *object)
{
    if ( dragged_module )
        dragged_module = NULL;
}

