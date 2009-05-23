
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

    /* Destination. */
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


/* gmsknavbar.c */
void gmsk_paint_navbar(cairo_t *cr);


/* globals */
extern GMutex *lock_for_model;
extern MskContainer *root_container, *current_container;

