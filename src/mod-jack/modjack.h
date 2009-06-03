typedef void (*ProcessCallback)(float *in, float *out_left, float *out_right,
        int nframes, int sample_rate, void *data);
typedef void (*EventCallback)(int nframes, int type, void *event_data,
        int event_size, void *data);

void *modjack_init(ProcessCallback process_func, EventCallback event_func,
                   void *arg, char **errmsg);
void modjack_fini(void *state);
void modjack_activate(void *state);
void modjack_deactivate(void *state);

