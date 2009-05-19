#include <glib/gtypes.h>

#ifdef WIN32
 #ifdef LIBMSK_EXPORTS
  #define MSK_API __declspec(dllexport)
 #else
  #define MSK_API __declspec(dllimport)
 #endif
#else
# define MSK_API
#endif


#define MSK_AUDIO_DATA      1
#define MSK_FLOAT_PROPERTY  2
#define MSK_INT_PROPERTY    3

typedef struct _MskWorld MskWorld;
typedef struct _MskContainer MskContainer;
typedef struct _MskInstrument MskInstrument;
typedef struct _MskPort MskPort;
typedef struct _MskModule MskModule;
typedef struct _MskProperty MskProperty;

typedef void (*MskProcessCallback)(MskModule *self, int start, int frames, void *state);
typedef void (*MskActivateCallback)(MskModule *self, void *state);
typedef void (*MskDeactivateCallback)(MskModule *self, void *state);
typedef void (*MskGlobalActivateCallback)(MskModule *self, void *state);
typedef void (*MskGlobalDeactivateCallback)(MskModule *self, void *state);
typedef void (*MskDynamicPortAddCallback)(MskModule *self);
typedef void (*MskDynamicPortRemoveCallback)(MskModule *self);

typedef void (*MskModuleLoadCallback)(GKeyFile *keyfile, MskModule *module, char *id);
typedef void (*MskModuleSaveCallback)(GKeyFile *keyfile, MskModule *module, char *id);


struct _MskWorld
{
    guint sample_rate;
    gsize block_size;

    MskContainer *root;

    GList *instruments;
};


struct _MskContainer
{
    MskModule *module;

    gboolean transparent;

    /* Internal port-modules. */
    MskModule **in;
    MskModule **out;

    GList *module_list;

    guint voices;
    guint voice_size;

    /* If the container is also an instrument. */
    MskInstrument *instrument;

    /* Runtime information. */
    GList *process_order;
    guint current_voice;
};


// Not sure if this should even exist
struct _MskInstrument
{
    MskContainer *container;

    int channel; //unused yet

    /* Voice information. */
    char *voice_active;
    short *voice_note;
    short *voice_velocity;

    int last_voice;

    /* A list of all MskParameters. */
    GList *parameter_list; //unused yet
};


struct _MskPort
{
    gchar *name;
    guint port_type;
    MskModule *owner;

    union
    {
        struct
        {
            GList *connections;
            MskPort *hardlink;
        } output;
        struct
        {
            MskPort *connection;
        } input;
    };

    gfloat default_value;
    void *buffer;
};


struct _MskModule
{
    gchar *name;
    MskWorld *world;
    MskContainer *parent;
    MskContainer *container;

    /* Callbacks */
    MskProcessCallback process;
    MskActivateCallback activate;
    MskDeactivateCallback deactivate;

    MskGlobalActivateCallback global_activate;
    MskGlobalDeactivateCallback global_deactivate;

    MskDynamicPortAddCallback dynamic_port_add;
    MskDynamicPortRemoveCallback dynamic_port_remove;

    GList *in_ports;
    GList *out_ports;
    GList *properties;

    GPtrArray *state;
    gsize state_size;
    void *global_state;
    gsize global_state_size;

    gboolean prepared;

    // TODO: this workaround must go away.
    MskPort *mix_to;

    /* Temporary; used when saving to file. */
    int save_id;
};

struct _MskProperty
{
    gchar *name;
    guint type;
    gpointer value;
    MskModule *owner;
};


/*** Functions ***/
void MSK_API msk_module_activate(MskModule *mod);
void MSK_API msk_module_deactivate(MskModule *mod);

void MSK_API msk_module_set_float_property(MskModule *mod, gchar *name, gfloat value);

gconstpointer MSK_API msk_module_get_property_buffer(MskModule *mod, gchar *name);
gconstpointer MSK_API msk_module_get_input_buffer(MskModule *mod, gchar *name);
gpointer      MSK_API msk_module_get_output_buffer(MskModule *mod, gchar *name);

void MSK_API msk_connect_ports(MskModule *left, gchar *left_port_name,
                       MskModule *right, gchar *right_port_name);

MskContainer MSK_API *msk_container_create(MskContainer *parent);
MskContainer MSK_API *msk_instrument_create(MskContainer *parent);

MskModule MSK_API *msk_input_create_with_name(MskContainer *parent, gchar *name, guint type);
MskModule MSK_API *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type);

MskModule MSK_API *msk_input_create(MskContainer *parent);
MskModule MSK_API *msk_output_create(MskContainer *parent);
MskModule MSK_API *msk_voice_create(MskContainer *parent);
MskModule MSK_API *msk_constant_create(MskContainer *parent);
MskModule MSK_API *msk_autoconstant_create(MskContainer *parent);
MskModule MSK_API *msk_oscillator_create(MskContainer *parent);
MskModule MSK_API *msk_pitchtofrequency_create(MskContainer *parent);
MskModule MSK_API *msk_addmul_create(MskContainer *parent);
MskModule MSK_API *msk_add_create(MskContainer *parent);
MskModule MSK_API *msk_mul_create(MskContainer *parent);
MskModule MSK_API *msk_voiceactive_create(MskContainer *parent);
MskModule MSK_API *msk_voicepitch_create(MskContainer *parent);
MskModule MSK_API *msk_voicevelocity_create(MskContainer *parent);
MskModule MSK_API *msk_adsr_create(MskContainer *parent);


MskContainer MSK_API *msk_world_create(gulong sample_rate, gsize block_size);
void MSK_API msk_world_destroy(MskContainer *world);
void MSK_API msk_world_prepare(MskContainer *container);
void MSK_API msk_world_run(MskContainer *container);
void MSK_API msk_message_note_on(MskWorld *world, short note, short velocity);
void MSK_API msk_message_note_off(MskWorld *world, short note, short velocity);

MskModule MSK_API *msk_factory_create_module(const char *name, MskContainer *parent);
MskContainer MSK_API *msk_load_world_from_file(const gchar *filename,
        MskModuleLoadCallback moduleload_callback, GError **error);
gboolean MSK_API msk_save_world_to_file(MskContainer *container, const gchar *filename,
        MskModuleSaveCallback modulesave_callback, GError **error);

// TODO: delete
void MSK_API msk_create_buffers_on_module(MskModule *module);
void MSK_API msk_destroy_buffers_on_module(MskModule *module);
void MSK_API msk_container_activate(MskContainer *self);
void MSK_API msk_container_deactivate(MskContainer *self);
