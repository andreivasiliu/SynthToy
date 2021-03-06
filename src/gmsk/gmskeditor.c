#include <glib.h>
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#include "msk0/msk0.h"
#include "gmsk.h"
#include "gmskinternal.h"


/* WARNING: This code might be abusing a bug in Pango. */

/* WARNING 2: The majority of this code will eventually be rewritten.
 * This is just a quick-and-dirty implementation, to help with tests.
 */

cairo_surface_t *surface;

GList *graphical_modules;

GraphicalModule *dragged_module;
int drag_grip_x;
int drag_grip_y;

GMPort *dragged_port;
int dragged_port_is_output;
int dragging_port_to_x;
int dragging_port_to_y;

int current_mouse_x;
int current_mouse_y;


GraphicalModule *auto_extended_module;
GraphicalModule *selected_module;
GMPort *selected_connection;

extern PangoLayout *create_pango_layout(cairo_t *cr, char *string,
                          int *width, int *height);
extern void paint_pango_layout(cairo_t *cr, PangoLayout *layout);


void gmsk_invalidate()
{
    if ( invalidate_callback )
        invalidate_callback(invalidate_userdata);
}

void gmsk_error_message(gchar *message)
{
    if ( error_message_callback )
        error_message_callback(message, error_message_userdata);
}

void draw_module(MskModule *mod, long x, long y)
{
    GraphicalModule *gmod;
    PangoLayout *title, *port;
    cairo_surface_t *surface;
    cairo_t *cr;
    int height, width, current_height;
    int surface_height, surface_width;
    GList *ports_left = NULL, *ports_right = NULL;
    GList *iter_left, *iter_right;
    GMPort *gmport_left, *gmport_right;

    gmod = g_new0(GraphicalModule, 1);

    surface = cairo_image_surface_create_for_data(NULL,
                                                  CAIRO_FORMAT_ARGB32,
                                                  0, 0, 0);

    cr = cairo_create(surface);

    /* Before we start drawing, we need to determine the minimum size of the
     * surface. */

    /* First, the border. */
    surface_width = 6;
    surface_height = 6;

    /* The title/name. */

    title = create_pango_layout(cr, mod->name, &width, &height);

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
    gmport_right = gmod->out_ports;

    while ( iter_left || iter_right )
    {
        MskPort *mport;
        int left_height = 0, left_width = 0;
        int right_height = 0, right_width = 0;

        if ( iter_left )
        {
            mport = iter_left->data;

            port = create_pango_layout(cr, mport->name, &left_width, &left_height);
            left_width += 8;
            left_height = MAX(left_height, 7);

            gmport_left->owner = gmod;
            gmport_left->port = mport;

            ports_left = g_list_append(ports_left, port);
            iter_left = g_list_next(iter_left);
            gmport_left++;
        }

        if ( iter_right )
        {
            mport = iter_right->data;

            port = create_pango_layout(cr, mport->name, &right_width, &right_height);
            right_width += 8;
            right_height = MAX(right_height, 7);

            gmport_right->owner = gmod;
            gmport_right->port = mport;

            ports_right = g_list_append(ports_right, port);
            iter_right = g_list_next(iter_right);
            gmport_right++;
        }

        surface_height += MAX(left_height, right_height);
        surface_width = MAX(surface_width,
                            left_width + right_width + 6 + 6);
    }

//    /* Let's say, space for some ports */
//    surface_height += height*2;

    gmod->width = surface_width;
    gmod->height = surface_height;

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

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

    pango_cairo_update_layout(cr, title);
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
            pango_cairo_update_layout(cr, port);
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
            pango_cairo_update_layout(cr, port);
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

    if ( x == -1 || y == -1 )
    {
        x = current_mouse_x - gmod->width / 2;
        y = current_mouse_y - gmod->height / 2;
    }

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


void get_absolute_gmport_position(GMPort *gmport, int *x, int *y)
{
    *x = gmport->owner->x + gmport->pos_x + 0.5;
    *y = gmport->owner->y + gmport->pos_y + 0.5;
}

GMPort *get_peer_gmport(GMPort *input_gmport)
{
    MskPort *dest_port;
    GraphicalModule *gmod;
    int nr;

    dest_port = input_gmport->port->input.connection;
    if ( !dest_port )
        return NULL;

    gmod = find_gmod(dest_port->owner);
    if ( !gmod )
        return NULL;

    // This looks messy.. maybe it should be changed.
    nr = g_list_index(gmod->mod->out_ports, dest_port);
    return &gmod->out_ports[nr];
}


void redraw_module(MskModule *mod)
{
    GraphicalModule *gmod;

    gmod = find_gmod(mod);
    if ( !gmod )
        return;

    // TODO: Memory leak.
    graphical_modules = g_list_remove(graphical_modules, gmod);
    draw_module(mod, gmod->x, gmod->y);
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

        if ( gmod->mod->parent != current_container )
            continue;

        gmport = gmod->in_ports;
        for ( i = 0; i < gmod->in_ports_nr; i++, gmport++ )
        {
            MskPort *dest_port = gmport->port->input.connection;

            if ( dest_port )
            {
                int nr;
                double src_x;
                double src_y;
                double dest_x;
                double dest_y;


                /* Find the destination. */
                dest_gmod = find_gmod(dest_port->owner);

                if ( !dest_gmod )
                    continue;

                nr = g_list_index(dest_port->owner->out_ports, dest_port);
                dest_gmport = &dest_gmod->out_ports[nr];

                src_x = gmod->x + gmport->pos_x + 0.5;
                src_y = gmod->y + gmport->pos_y + 0.5;
                dest_x = dest_gmod->x + dest_gmport->pos_x + 0.5;
                dest_y = dest_gmod->y + dest_gmport->pos_y + 0.5;

                cairo_move_to(cr, src_x, src_y);
                cairo_curve_to(cr,
                               src_x - 20, src_y,
                               dest_x + 20, dest_y,
                               dest_x, dest_y);
                if ( gmport == selected_connection )
                    cairo_set_source_rgb(cr, 1, 1, 1);
                else
                    cairo_set_source_rgb(cr, 1, 0.8, 0.8);
                cairo_set_line_width(cr, 1);
                cairo_stroke(cr);
            }
        }
    }
}


void gmsk_paint_editor(cairo_t *cr)
{
    GList *item;

    //g_print("Paint requested.\n");

    cairo_set_source_rgb(cr, 0.2, 0.2, 0.3);
    cairo_paint(cr);

    for ( item = graphical_modules; item; item = item->next )
    {
        GraphicalModule *gmod = item->data;

        if ( gmod->mod->parent != current_container )
            continue;

        cairo_set_source_surface(cr, gmod->surface, gmod->x, gmod->y);

//        if ( gmod == dragged_module )
//            cairo_paint_with_alpha(cr, 0.8);
//        else
            cairo_paint(cr);

        if ( gmod == selected_module )
        {
            cairo_save(cr);
            cairo_set_line_width(cr, 1);
            cairo_translate(cr, gmod->x, gmod->y);

            cairo_rectangle(cr, 1.5, 1.5, gmod->width - 3, gmod->height - 3);
            cairo_set_source_rgb(cr, 0.4, 0, 0.6);
            cairo_stroke(cr);

            cairo_restore(cr);
        }
    }

    draw_connections(cr);

    if ( dragged_port )
    {
        double dashes[] = { 5, 5 };

        /* Draw a bezier from the source. */
        double src_x, src_y;
        double dest_x, dest_y;

        src_x = dragged_port->owner->x + dragged_port->pos_x + 0.5;
        src_y = dragged_port->owner->y + dragged_port->pos_y + 0.5;
        dest_x = dragging_port_to_x + 0.5;
        dest_y = dragging_port_to_y + 0.5;

        cairo_save(cr);

        cairo_move_to(cr, src_x, src_y);
        if ( dragged_port_is_output )
            cairo_curve_to(cr,
                           src_x + 20, src_y,
                           dest_x - 20, dest_y,
                           dest_x, dest_y);
        else
            cairo_curve_to(cr,
                           src_x - 20, src_y,
                           dest_x + 20, dest_y,
                           dest_x, dest_y);
        cairo_set_source_rgb(cr, 1, 0.8, 0.8);
        cairo_set_dash(cr, dashes, 2, 0);
        cairo_set_line_width(cr, 1);
        cairo_stroke(cr);

        cairo_restore(cr);
    }

    gmsk_paint_navbar(cr);

    cairo_destroy(cr);
}


GraphicalModule *get_gmod_at(int x, int y)
{
    GraphicalModule *gmod;
    GList *item;

    /* The last one painted is the first one reachable. */
    item = g_list_last(graphical_modules);
    while ( item )
    {
        gmod = item->data;

        if ( gmod->mod->parent == current_container &&
             x >= gmod->x && x < gmod->x + gmod->width &&
             y >= gmod->y && y < gmod->y + gmod->height )
            return gmod;

        item = item->prev;
    }

    return NULL;
}


GMPort *get_gmport_at(GraphicalModule *gmod, int x, int y, int *type)
{
    int i;

    for ( i = 0; i < gmod->in_ports_nr; i++ )
    {
        GMPort *gmport = &gmod->in_ports[i];

        if ( x > gmport->pos_x - 8 && x < gmport->pos_x + 16 &&
             y > gmport->pos_y - 8 && y < gmport->pos_y + 8 )
        {
            if ( type )
                *type = 1;

            return gmport;
        }
    }

    for ( i = 0; i < gmod->out_ports_nr; i++ )
    {
        GMPort *gmport = &gmod->out_ports[i];

        if ( x > gmport->pos_x - 16 && x < gmport->pos_x + 8 &&
             y > gmport->pos_y - 8 && y < gmport->pos_y + 8 )
        {
            if ( type )
                *type = 2;

            return gmport;
        }
    }

    if ( type )
        *type = 0;

    return NULL;
}

GMPort *get_connection_at(int x, int y)
{
    GMPort *output_gmport, *input_gmport;
    GList *item;
    int i;

    for ( item = graphical_modules; item; item = item->next )
    {
        GraphicalModule *gmod = item->data;

        if ( gmod->mod->parent != current_container )
            continue;

        input_gmport = gmod->in_ports;
        for ( i = 0; i < gmod->in_ports_nr; i++, input_gmport++ )
        {
            int src_x, src_y, dest_x, dest_y;

            output_gmport = get_peer_gmport(input_gmport);
            if ( !output_gmport )
                continue;

            get_absolute_gmport_position(output_gmport, &src_x, &src_y);
            get_absolute_gmport_position(input_gmport, &dest_x, &dest_y);

            if ( is_point_near_bezier_curve(x, y,
                    src_x, src_y,
                    src_x + 20, src_y,
                    dest_x - 20, dest_y,
                    dest_x, dest_y) )
                return input_gmport;
        }
    }

    return NULL;
}

void gmsk_connect_gmports(GMPort *output, GMPort *input)
{
    GError *error = NULL;

    gmsk_lock_mutex();

    msk_container_deactivate(root_container);
    msk_try_connect_ports(output->owner->mod, output->port->name,
                          input->owner->mod, input->port->name, &error);
    msk_container_activate(root_container);

    gmsk_unlock_mutex();

    if ( error )
    {
        gmsk_error_message(error->message);
        g_error_free(error);
    }
}

MskModule *msk_get_selected_module()
{
    if ( selected_module )
        return selected_module->mod;

    return NULL;
}

MskPort *msk_get_selected_connection()
{
    if ( selected_connection )
        return selected_connection->port;

    return NULL;
}

gboolean gmsk_mouse_motion_event(int x, int y, int modifiers)
{
    current_mouse_x = x;
    current_mouse_y = y;

    if ( dragged_module )
    {
        long new_x, new_y;

        new_x = x - drag_grip_x;
        new_y = y - drag_grip_y;

        /* Unless 'shift' is pressed, snap to a 5x5 grid. */
        if ( !(modifiers & GMSK_SHIFT_MASK) )
        {
            /* I wrote this due to lack of inspiration... How else do I round it? */
            new_x = ((new_x + 2) - (new_x + 2) % 5);
            new_y = ((new_y + 2) - (new_y + 2) % 5);
        }

        if ( new_x < 0 )
            new_x = 0;
        if ( new_y < 0 )
            new_y = 0;

        /* This prevents redraws when nothing changed. */
        if ( dragged_module->x != new_x ||
             dragged_module->y != new_y )
        {
            dragged_module->x = new_x;
            dragged_module->y = new_y;

            gmsk_invalidate();
        }

        return TRUE;
    }

    if ( dragged_port )
    {
        GraphicalModule *gmod = get_gmod_at(x, y);

        dragging_port_to_x = x;
        dragging_port_to_y = y;

        /* Dragging over a module? Auto-extend it. */
        if ( dragged_port_is_output )
        {
            if ( gmod && gmod != dragged_port->owner )
            {
                gmsk_lock_mutex();

                if ( gmod != auto_extended_module )
                {
                    msk_module_add_dynamic_ports(gmod->mod);
                    redraw_module(gmod->mod);
                    // TODO: this is because in this quick-and-dirty implementation
                    // the gmod is replaced when redrawn.
                    auto_extended_module = find_gmod(gmod->mod);
                }

                gmsk_unlock_mutex();
            }
            else if ( auto_extended_module )
            {
                gmsk_lock_mutex();

                msk_module_remove_unused_dynamic_ports(auto_extended_module->mod);
                redraw_module(auto_extended_module->mod);
                auto_extended_module = NULL;

                gmsk_unlock_mutex();
            }
        }

        gmsk_invalidate();
    }

    return FALSE;
}


void gmsk_select_module(GraphicalModule *gmod)
{
    if ( selected_connection )
        gmsk_select_connection(NULL);

    /* Select it, and bring it to the top. */
    selected_module = gmod;

    if ( selected_module )
    {
        graphical_modules = g_list_remove(graphical_modules, gmod);
        graphical_modules = g_list_append(graphical_modules, gmod);
    }

    if ( select_module_callback )
        select_module_callback(gmod ? gmod->mod : NULL, select_module_userdata);
}

void gmsk_select_connection(GMPort *connection)
{
    if ( selected_module )
        gmsk_select_module(NULL);

    selected_connection = connection;

    /* Maybe there will be a callback here... but for now we don't need it. */
}

gboolean gmsk_mouse_press_event(int x, int y, int button, int type, int modifiers)
{
    GraphicalModule *gmod;
    GMPort *connection;

    /* Only left mouse button can select and drag. */
    if ( button != 1 )
        return FALSE;

    /* The last one painted is the first one reachable. */
    gmod = get_gmod_at(x, y);
    if ( gmod )
    {
        GMPort *gmport;
        int gmport_type;

        /* Bring it to the top and show its properties. */
        gmsk_select_module(gmod);

        /* Did we click on a port of the module? */
        gmport = get_gmport_at(gmod, x - gmod->x, y - gmod->y, &gmport_type);
        if ( gmport )
        {
            dragged_port = gmport;
            dragged_port_is_output = (gmport_type == 2);
            dragging_port_to_x = x;
            dragging_port_to_y = y;

            return TRUE;
        }

        // Print some info about it.
        g_print("Module: '%s', at x:%d, y:%d.\n", gmod->mod->name,
                (int) gmod->x, (int) gmod->y);

        /* Double-click on a container? */
        if ( type == GMSK_2BUTTON_PRESS )
        {
            if ( gmod->mod->container )
                current_container = gmod->mod->container;
        }
        else
        {
            dragged_module = gmod;
            drag_grip_x = x - gmod->x;
            drag_grip_y = y - gmod->y;
        }

        gmsk_invalidate();

        return TRUE;
    }

    /* Did we click on a connection? */
    connection = get_connection_at(x, y);
    if ( connection )
    {
        gmsk_select_connection(connection);
        gmsk_invalidate();

        return TRUE;
    }

    /* Click on empty space deselects a connection. */
    if ( !connection )
    {
        gmsk_select_connection(NULL);
        gmsk_invalidate();
    }

    /* Double-click on empty space? */
    if ( type == GMSK_2BUTTON_PRESS &&
         current_container->module->parent )
    {
        current_container = current_container->module->parent;
        gmsk_invalidate();

        return TRUE;
    }

    return FALSE;
}

gboolean gmsk_mouse_release_event(int x, int y, int button, int modifiers)
{
    if ( dragged_module )
    {
        dragged_module = NULL;
        gmsk_invalidate();
    }

    if ( dragged_port )
    {
        GraphicalModule *gmod;
        GMPort *gmport;
        int type;

        gmod = get_gmod_at(x, y);

        if ( gmod )
        {
            gmport = get_gmport_at(gmod, x - gmod->x, y - gmod->y, &type);

            if ( gmport )
            {
                g_print("Dropped into a GMPort!\n");

                if ( dragged_port_is_output && type == 1 )
                    gmsk_connect_gmports(dragged_port, gmport);
                else if ( !dragged_port_is_output && type == 2 )
                    gmsk_connect_gmports(gmport, dragged_port);

                g_print("Source: %s.\n", dragged_port->port ? dragged_port->port->name : "-");
                g_print("Dest: %s.\n", gmport->port ? gmport->port->name : "-");
            }
        }

        dragged_port = NULL;
        gmsk_invalidate();
    }

    if ( auto_extended_module )
    {
        gmsk_lock_mutex();
        msk_module_remove_unused_dynamic_ports(auto_extended_module->mod);
        redraw_module(auto_extended_module->mod);
        gmsk_unlock_mutex();
        auto_extended_module = NULL;
    }

    return FALSE;
}

MskModule *gmsk_get_selected_module()
{
    if ( selected_module )
        return selected_module->mod;

    return NULL;
}

void gmsk_delete_selected()
{
    gmsk_lock_mutex();

    if ( selected_module )
    {
        GraphicalModule *doomed_gmod = selected_module;

        gmsk_select_module(NULL);
        graphical_modules = g_list_remove(graphical_modules, doomed_gmod);
        // TODO: free it.
        msk_module_destroy(doomed_gmod->mod);

        gmsk_invalidate();
    }

    if ( selected_connection )
    {
        GMPort *doomed_connection = selected_connection;

        gmsk_select_connection(NULL);
        msk_disconnect_input_port(doomed_connection->port);
        if ( msk_module_remove_unused_dynamic_ports(doomed_connection->port->owner) )
            redraw_module(doomed_connection->owner->mod);

        gmsk_invalidate();
    }

    gmsk_unlock_mutex();
}

