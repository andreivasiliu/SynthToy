
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


/* gmsksaveload.c */

gboolean gmsk_save_world_to_file(const gchar *filename, GError **error);
gboolean gmsk_load_world_from_file(const gchar *filename, GError **error);


/* gmskeditor.c */
// TODO: these should be changed into something better
GraphicalModule *find_gmod(MskModule *mod);
void draw_module(MskModule *mod, long x, long y);

