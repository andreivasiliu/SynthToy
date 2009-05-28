#include <glib.h>
#include <cairo.h>

#include "msk0/msk0.h"
#include "gmsk.h"
#include "gmskinternal.h"


gboolean is_point_near_bezier_curve(int x, int y,
        int p1x, int p1y, int q1x, int q1y, int q2x, int q2y, int p2x, int p2y)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    unsigned char *pixel = NULL;
    int stride;

    /* Create a memory surface of just 1 pixel. (1x1) */
    stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, 1);
    pixel = g_alloca(stride);
    surface = cairo_image_surface_create_for_data(pixel, CAIRO_FORMAT_RGB24, 1, 1, stride);
    cr = cairo_create(surface);

    /* Translate so that our solitary pixel coincides with the real pixel at
     * position [x,y]. */
    cairo_translate(cr, -x, -y);

    /* Initialize with black. */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    /* Draw the curve. */
    cairo_move_to(cr, p1x, p1y);
    cairo_curve_to(cr, q1x, q1y, q2x, q2y, p2x, p2y);

    cairo_set_line_width(cr, 20);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke(cr);

    /* Clean up. */
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    /* Did the curve pass through our pixel? */
    return pixel[0] != 0;
}

