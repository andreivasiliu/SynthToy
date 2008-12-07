#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* These includes would ideally go away. */
#include <gtk/gtk.h>
#include <cairo.h>

extern void refresh_osc_window();

extern int get_my_size();
extern float *get_my_array();
extern void *init_jack_client();
extern void close_jack_client();
extern int get_current_frames(int nframes, float *dest, void *arg);

extern int aural_process(float *in, float *out, int nframes, void *instance);
extern void *new_aural_instance(int frame_rate);
extern void free_aural_instance(void *instance);

extern void aural_init();
extern void process_func(float *in, float *out, int nframes, int sample_rate, void *data);
extern void event_func(int nframes, int type, void *event_data, int event_size, void *data);

/* gmskmenu.c */
extern GtkWidget *gmsk_create_menu();

/* gmsknavbar.c */
extern void gmsk_paint_navbar(cairo_t *cr);
