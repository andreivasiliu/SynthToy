
/* Structures. */

typedef struct _GraphicalModule GraphicalModule;
typedef struct _GMPort GMPort;

struct _GMPort
{
    GraphicalModule *owner;
    MskPort *port;

    /* Port's position on the module. */
    int pos_x;
    int pos_y;
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


/* gmskmain.c */
extern GmskInvalidateCallback invalidate_callback;
extern GmskSelectModuleCallback select_module_callback;
extern gpointer invalidate_userdata, select_module_userdata;
extern MskContainer *root_container, *current_container;


/* gmskeditor.c */
// TODO: these should be changed into something better
GraphicalModule *find_gmod(MskModule *mod);
void draw_module(MskModule *mod, long x, long y);
void redraw_module(MskModule *mod);
void gmsk_invalidate();
void gmsk_select_connection(GMPort *connection);
void gmsk_select_module(GraphicalModule *gmod);


/* gmsknavbar.c */
void gmsk_paint_navbar(cairo_t *cr);


/* gmskselect.c */
gboolean is_point_near_bezier_curve(int x, int y,
        int p1x, int p1y, int q1x, int q1y, int q2x, int q2y, int p2x, int p2y);


/* globals */
extern GMutex *lock_for_model;
extern MskContainer *root_container, *current_container;

