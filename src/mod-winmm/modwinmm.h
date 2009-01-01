
#ifdef WIN32
 #ifdef MODWINMM_EXPORTS
  #define MW_API __declspec(dllexport)
 #else
  #define MW_API __declspec(dllimport)
 #endif
#else
# define MW_API 
#endif

typedef void MW_API (*ProcessCallback)(float *in, float *out, int nframes,
                                int sample_rate, void *data);
typedef void MW_API (*EventCallback)(int nframes, int type, void *event_data,
                              int event_size, void *data);

void MW_API *modwinmm_init(ProcessCallback process_func, EventCallback event_func,
                    void *arg, char **errmsg);
void MW_API modwinmm_fini(void *state);
void MW_API modwinmm_activate(void *state);
void MW_API modwinmm_deactivate(void *state);

