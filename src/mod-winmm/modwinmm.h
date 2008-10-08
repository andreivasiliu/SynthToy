typedef void (*ProcessCallback)(float *in, float *out, int nframes,
                                int sample_rate, void *data);
typedef void (*EventCallback)(int nframes, int type, void *event_data,
                              int event_size, void *data);

void *modwinmm_init(ProcessCallback process_func, EventCallback event_func,
                    void *arg, char **errmsg);
void modwinmm_fini(void *state);
void modwinmm_activate(void *state);
void modwinmm_deactivate(void *state);

