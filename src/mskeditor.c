#include <gtk/gtk.h>
#include <cairo.h>
#define PANGO_ENABLE_BACKEND 1
#include <pango/pango.h>
#include <pango/pangocairo.h>
#undef PANGO_ENABLE_BACKEND

#include <libintl.h>

#include "msk0/msk0.h"


/* WARNING: This code might be abusing a bug in Pango. */

/* WARNING 2: The majority of this code will eventually be rewritten.
 * This is just a quick-and-dirty implementation, to help with tests.
 */

cairo_surface_t *surface;

typedef struct _GraphicalModule GraphicalModule;
typedef struct _GMPort GMPort;

struct _GMPort
{
    MskPort *port;
    
    /* Port's position on the module. */
    int pos_x;
    int pos_y;
    
    /* Destination. */
    GraphicalModule *dest_gmod;
    int dest_port_nr;
};

struct _GraphicalModule
{
    cairo_surface_t *surface;
    MskModule *mod;
    
    long x, y;
    
    long width;
    long height;
    
    GMPort *in_ports;
    GMPort *out_ports;
    int in_ports_nr;
    int out_ports_nr;
};


GList *graphical_modules;

GraphicalModule *dragged_module;
int drag_grip_x;
int drag_grip_y;


void draw_module(MskModule *mod, long x, long y)
{
    GraphicalModule *gmod;
    PangoContext *context;
    PangoFontMap *font_map;
    PangoLayout *title, *port;
    PangoFontDescription *font_desc;
    cairo_surface_t *surface;
    cairo_t *cr;
    int height, width, current_height;
    int surface_height, surface_width;
    GList *ports_left = NULL, *ports_right = NULL;
    GList *iter_left, *iter_right;
    GMPort *gmport_left, *gmport_right;
    
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
    
    pango_layout_set_text(title, mod->name, -1);
    
    pango_layout_get_pixel_size(title, &width, &height);
    
    surface_width += width+1 + 5*2; /* 5 == padding */
    surface_height += height;
    
    /* Separator line */
    surface_height += 3;
    
    /* Prepare ports. */
    gmod->in_ports_nr = g_list_length(mod->in_ports);
    gmod->out_ports_nr = g_list_length(mod->out_ports);
    gmod->in_ports = g_new0(GMPort, gmod->in_ports_nr);
    gmod->out_ports = g_new0(GMPort, gmod->out_ports_nr);
    
    /* Ports */
    iter_left = mod->in_ports;
    iter_right = mod->out_ports;
    gmport_left = gmod->in_ports;
    
    while ( iter_left || iter_right )
    {
        MskPort *mport;
        int left_height = 0, left_width = 0;
        int right_height = 0, right_width = 0;
        
        if ( iter_left )
        {
            mport = iter_left->data;
            
            port = pango_layout_new(context);
            pango_layout_set_font_description(port, font_desc);
            pango_layout_set_text(port, mport->name, -1);
            pango_layout_get_pixel_size(port, &left_width, &left_height);
            left_width += 8;
            left_height = MAX(left_height, 7);
            
            gmport_left->port = mport;
            
            ports_left = g_list_append(ports_left, port);
            iter_left = g_list_next(iter_left);
            gmport_left++;
        }
        
        if ( iter_right )
        {
            mport = iter_right->data;
            
            port = pango_layout_new(context);
            pango_layout_set_font_description(port, font_desc);
            pango_layout_set_text(port, mport->name, -1);
            pango_layout_get_pixel_size(port, &right_width, &right_height);
            right_width += 8;
            right_height = MAX(right_height, 7);
            
            ports_right = g_list_append(ports_right, port);
            iter_right = g_list_next(iter_right);
        }
        
        surface_height += MAX(left_height, right_height);
        surface_width = MAX(surface_width,
                            left_width + right_width + 6 + 6);
    }
    
//    /* Let's say, space for some ports */
//    surface_height += height*2;
    
    pango_font_description_free(font_desc);
    
    gmod->width = surface_width;
    gmod->height = surface_height;
    
    /* Now that we have the required size, create a surface and start drawing
     * on it. */
    
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         surface_width, surface_height);
    
    cr = cairo_create(surface);
    
    current_height = 0;
    
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
    
    current_height += 3;
    
    /* Paint the title */
    cairo_set_source_rgb(cr, 1, 1, 1);
    
    pango_layout_get_pixel_size(title, &width, &height);
    cairo_move_to(cr, (surface_width - width) / 2, 4);
    pango_cairo_show_layout(cr, title);
    
    current_height += height;
    
    /* Paint the separator */
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_move_to(cr, 3, 4.5 + height);
    cairo_line_to(cr, surface_width - 3, 4.5 + height);
    cairo_stroke(cr);
    
    current_height += 3;
    
    /* Paint the ports */
    iter_left = ports_left;
    iter_right = ports_right;
    gmport_left = gmod->in_ports;
    gmport_right = gmod->out_ports;
    
    while ( iter_left || iter_right )
    {
        int left_width = 0, right_width = 0;
        int left_height = 0, right_height = 0;
        
        if ( iter_left )
        {
            port = iter_left->data;
            pango_layout_get_pixel_size(port, &left_width, &left_height);
            
            /* Paint a little box. */
            cairo_set_source_rgb(cr, 1, 0, 0);
            cairo_rectangle(cr, 5, current_height + left_height/2-2,
                            5, 5);
            cairo_fill(cr);
            
            /* Remember the box's position. */
            gmport_left->pos_x = 7;
            gmport_left->pos_y = current_height + left_height/2;
            
            /* Paint the port name. */
            cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
            cairo_move_to(cr, 3+8+1, current_height);
            pango_cairo_show_layout(cr, port);
            
            left_height = MAX(left_height, 7);
            iter_left = g_list_next(iter_left);
            gmport_left++;
        }
        
        if ( iter_right )
        {
            port = iter_right->data;
            pango_layout_get_pixel_size(port, &right_width, &right_height);
            
            /* Paint a little box. */
            cairo_set_source_rgb(cr, 0, 0, 1);
            cairo_rectangle(cr, surface_width - 10,
                            current_height + right_height/2-2,
                            5, 5);
            cairo_fill(cr);
            
            /* Remember the box's position. */
            gmport_right->pos_x = surface_width - 8;
            gmport_right->pos_y = current_height + right_height/2;
            
            /* Paint the port name. */
            cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
            cairo_move_to(cr, surface_width-3-8-right_width, current_height);
            pango_cairo_show_layout(cr, port);
            
            right_height = MAX(right_height, 7);
            iter_right = g_list_next(iter_right);
            gmport_right++;
        }
        
        current_height += MAX(left_height, right_height);
    }
    
    cairo_destroy(cr);
    
    gmod->surface = surface;
    gmod->mod = mod;
    gmod->x = x;
    gmod->y = y;
    
    graphical_modules = g_list_append(graphical_modules, gmod);
}


GraphicalModule *find_gmod(MskModule *mod)
{
    GList *item;
    
    for ( item = graphical_modules; item; item = item->next )
    {
        GraphicalModule *gmod = item->data;
        
        if ( gmod->mod == mod )
            return gmod;
    }
    
    return NULL;
}


void draw_connections(cairo_t *cr)
{
    GList *item;
    GraphicalModule *gmod, *dest_gmod;
    GMPort *gmport, *dest_gmport;
    int i;
    
    /* We draw from an in-port to an out-port... because an in-port
     * has only one connection to it. */
    
    for ( item = graphical_modules; item; item = item->next )
    {
        gmod = item->data;
        
        gmport = gmod->in_ports;
        for ( i = 0; i < gmod->in_ports_nr; i++, gmport++ )
        {
            MskPort *dest_port = gmport->port->input.connection;
            
            if ( dest_port )
            {
                int nr;
                
                /* Find the destination. */
                dest_gmod = find_gmod(dest_port->owner);
                
                if ( !dest_gmod )
                    continue;
                
                nr = g_list_index(dest_port->owner->out_ports, dest_port);
                dest_gmport = &dest_gmod->out_ports[nr];
                
	        long src_x = gmod->x + gmport->pos_x + 0.5;
	        long src_y = gmod->y + gmport->pos_y + 0.5;
		long dest_x = dest_gmod->x + dest_gmport->pos_x + 0.5;
	        long dest_y = dest_gmod->y + dest_gmport->pos_y + 0.5;
	        
                cairo_move_to(cr, src_x, src_y);
                cairo_curve_to(cr,
			       src_x - 20, src_y,
			       dest_x + 20, dest_y,
			       dest_x, dest_y);
                cairo_set_source_rgb(cr, 1, 0.8, 0.8);
                cairo_set_line_width(cr, 1);
                cairo_stroke(cr);
            }
        }
    }
}


void paint_editor(GtkWidget *widget)
{
    GList *item;
    cairo_t *cr;
    
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
    
    draw_connections(cr);
    
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
        
        /* This causes redraws even when nothing moved.. :( */
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
    {
        dragged_module = NULL;
        gtk_widget_queue_draw(GTK_WIDGET(object));
    }
}

