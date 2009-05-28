#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


// TODO: Old.. sort this out.
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

/* first.c */
void paint_array_to_widget(GtkWidget *widget, float *array, int length);
extern GtkWidget *main_window;
extern GtkWidget *left_osc, *editor, *virkb, *vscale_pw, *vscale_mw;
extern GtkWidget *properties_frame;
extern GtkListStore *liststore_properties;

/* menu.c */
extern GtkWidget *create_menu();

/* process.c */
extern float array[512];
extern float array2[512];
extern gdouble processing_time;
MskContainer *aural_root;

/* virkb.c */
extern void paint_keyboard(GtkWidget *widget);

