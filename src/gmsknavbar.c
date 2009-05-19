#include <gtk/gtk.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#include "header.h"
#include "msk0/msk0.h"


extern MskContainer *current_container;

void set_pango_font_description(PangoLayout *layout)
{
    PangoFontDescription *desc;

    /* This will eventually become a setting, somewhere. */

    // TODO: It will also become a globally initialized value.

    desc = pango_font_description_from_string("Sans 8");
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

void get_pango_text_size(char *string, int *width, int *height)
{
    PangoLayout *layout;
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create_for_data(NULL,
                                                  CAIRO_FORMAT_ARGB32,
                                                  0, 0, 0);

    cr = cairo_create(surface);

    layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, string, -1);
    set_pango_font_description(layout);

    pango_layout_get_pixel_size(layout, width, height);
    g_object_unref(layout);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}


/* Create a layout to be painted later. */
PangoLayout *create_pango_layout(cairo_t *cr, char *string,
                          int *width, int *height)
{
    PangoLayout *layout;

    layout = pango_cairo_create_layout(cr);

    pango_layout_set_text(layout, string, -1);
    set_pango_font_description(layout);

    /* This accepts NULL too, so it's safe. */
    pango_layout_get_pixel_size(layout, width, height);

    return layout;
}


/* Paint and destroy a layout. */
void paint_pango_layout(cairo_t *cr, PangoLayout *layout)
{
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
}

void paint_pango_text(cairo_t *cr, char *string, int *width, int *height)
{
    PangoLayout *layout;

    layout = create_pango_layout(cr, string, width, height);
    paint_pango_layout(cr, layout);
}


/* Paint a single container. */
int gmsk_paint_navbar_item(cairo_t *cr, MskContainer *container)
{
    PangoLayout *layout;
    int width;
    int text_width, text_height;

    if ( container->module->parent )
        width = gmsk_paint_navbar_item(cr, container->module->parent);
    else
        width = 8;

    layout = create_pango_layout(cr, container->module->name,
                                 &text_width, &text_height);

    cairo_move_to(cr, width - 8, text_height);
    cairo_line_to(cr, width + text_width, text_height);
    cairo_line_to(cr, width + text_width + 8, 0);
    if ( width == 8 )
        cairo_line_to(cr, 0, 0);
    else
        cairo_line_to(cr, width, 0);
    cairo_close_path(cr);
    cairo_set_source_rgb(cr, 0.35, 0.35, 0.4);
    cairo_fill(cr);

    cairo_move_to(cr, width - 8, text_height + 0.5);
    cairo_line_to(cr, width + text_width + 0.5, text_height + 0.5);
    cairo_line_to(cr, width + text_width + 8.5, 0);
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_set_line_width(cr, 1.5);
    cairo_stroke(cr);

    cairo_move_to(cr, width, 0);
    cairo_set_source_rgb(cr, 1, 1, 1);
    paint_pango_layout(cr, layout);

    return width + text_width + 9;
}

void gmsk_paint_navbar(cairo_t *cr)
{
    cairo_save(cr);

    gmsk_paint_navbar_item(cr, current_container);

    cairo_restore(cr);
}
