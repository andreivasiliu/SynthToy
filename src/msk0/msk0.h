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


/* Port and property types. */
#define MSK_AUDIO_DATA          (1 << 0)
#define MSK_CONTROL_DATA        (1 << 1)
#define MSK_FLOAT_PROPERTY      (1 << 2)
#define MSK_INT_PROPERTY        (1 << 3)
#define MSK_STRING_PROPERTY     (1 << 4)

/* Port flags. */
#define MSK_INPUT_PORT          (1 << 0)
#define MSK_OUTPUT_PORT         (1 << 1)

/* Property flags. */
#define MSK_PROPERTY_DEACTIVATES_MODULE  (1 << 0)

typedef struct _MskWorld MskWorld;
typedef struct _MskContainer MskContainer;
typedef struct _MskInstrument MskInstrument;
typedef struct _MskPort MskPort;
typedef struct _MskModule MskModule;
typedef struct _MskProperty MskProperty;
typedef struct _MskProcessor MskProcessor;
typedef struct _MskBufferList MskBufferList;

typedef void (*MskProcessCallback)(MskModule *self, int start, int frames, void *state);
typedef void (*MskActivateCallback)(MskModule *self, void *state);
typedef void (*MskDeactivateCallback)(MskModule *self, void *state);
typedef void (*MskDynamicPortAddCallback)(MskModule *self);
typedef void (*MskDynamicPortRemoveCallback)(MskModule *self);
typedef void (*MskModuleDestroyCallback)(MskModule *self);

typedef void (*MskPropertyWriteCallback)(MskProperty *property, void *value);

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

    gsize block_size_limit;

    /* Internal port-modules. */
    MskModule **in;
    MskModule **out;

    GList *module_list;

    guint voices;
    guint voice_size;

    /* If the container is also an instrument. */
    MskInstrument *instrument;

    /* A list of 'tasks', of type MskProcessor. */
    GList *processing_list;

    /* This is needed by a container's children. */
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
    MskModule *owner;

    guint port_type;
    guint flags;
    gint group;

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

    MskModuleDestroyCallback destroy_callback;

    MskDynamicPortAddCallback dynamic_port_add;
    MskDynamicPortRemoveCallback dynamic_port_remove;
    guint dynamic_group_size;
    guint dynamic_group_count;

    GList *in_ports;
    GList *out_ports;
    GList *properties;

    struct
    {
        void ***group;
        guint *group_size;
        guint groups;
    } buffer_groups;

    GPtrArray *state;
    gsize state_size;
    void *global_state;

    /* This is for delays. When a delay is in a loop, it basically acts as two
     * separate modules that break the loop. */
    gboolean can_split;
    gboolean is_split;
    int delay_limiter;
    MskProcessCallback process_input;
    MskProcessCallback process_output;

    // TODO: this workaround must go away.
    MskPort *mix_to;

    /* Temporary fields. */

    /* Used for sorting topologically, and finding loops. */
    gboolean prepared;
    gboolean could_cause_loop;
    /* Used when saving to file. */
    int save_id;
};

struct _MskProperty
{
    gchar *name;
    MskModule *owner;

    guint type;
    guint flags;
    gpointer value;
    MskPropertyWriteCallback callback;
};


/*** Functions ***/
void MSK_API msk_module_activate(MskModule *mod);
void MSK_API msk_module_deactivate(MskModule *mod);
void MSK_API msk_module_reactivate(MskModule *mod);
void MSK_API msk_module_destroy(MskModule *mod);


/* Properties. */
void  MSK_API  msk_module_set_integer_property(MskModule *mod, gchar *name, gint value);
int   MSK_API  msk_module_get_integer_property(MskModule *mod, gchar *name);
void  MSK_API  msk_module_set_float_property(MskModule *mod, gchar *name, gfloat value);
float MSK_API  msk_module_get_float_property(MskModule *mod, gchar *name);
void  MSK_API  msk_module_set_string_property(MskModule *mod, gchar *name, gchar *value);
gchar MSK_API *msk_module_get_string_property(MskModule *mod, gchar *name);

MskProperty MSK_API *msk_module_get_property(MskModule *mod, gchar *prop_name);
void MSK_API msk_property_set_value_from_string(MskProperty *property, gchar *value);
gchar MSK_API *msk_property_get_value_as_string(MskProperty *property);

gconstpointer MSK_API msk_module_get_property_buffer(MskModule *mod, gchar *name);
gconstpointer MSK_API msk_module_get_input_buffer(MskModule *mod, gchar *name);
gpointer      MSK_API msk_module_get_output_buffer(MskModule *mod, gchar *name);


/* Ports. */
void MSK_API msk_connect_ports(MskModule *left, gchar *left_port_name,
                       MskModule *right, gchar *right_port_name);
void MSK_API msk_disconnect_input_port(MskPort *in_port);
void MSK_API msk_disconnect_output_port(MskPort *out_port);

void MSK_API msk_module_add_dynamic_ports(MskModule *module);
void MSK_API msk_module_remove_dynamic_ports(MskModule *module);
gboolean MSK_API msk_module_remove_unused_dynamic_ports(MskModule *module);


/* Module constructors. */
MskContainer MSK_API *msk_container_create(MskContainer *parent);
MskContainer MSK_API *msk_instrument_create(MskContainer *parent);

MskModule MSK_API *msk_input_create_with_name(MskContainer *parent, gchar *name, guint type);
MskModule MSK_API *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type);

MskModule MSK_API *msk_input_create(MskContainer *parent);
MskModule MSK_API *msk_output_create(MskContainer *parent);
MskModule MSK_API *msk_voicenumber_create(MskContainer *parent);
MskModule MSK_API *msk_constant_create(MskContainer *parent);
MskModule MSK_API *msk_oscillator_create(MskContainer *parent);
MskModule MSK_API *msk_pitchtofrequency_create(MskContainer *parent);
MskModule MSK_API *msk_addmul_create(MskContainer *parent);
MskModule MSK_API *msk_add_create(MskContainer *parent);
MskModule MSK_API *msk_mul_create(MskContainer *parent);
MskModule MSK_API *msk_voiceactive_create(MskContainer *parent);
MskModule MSK_API *msk_voicepitch_create(MskContainer *parent);
MskModule MSK_API *msk_voicevelocity_create(MskContainer *parent);
MskModule MSK_API *msk_parameter_create(MskContainer *parent);
MskModule MSK_API *msk_adsr_create(MskContainer *parent);
MskModule MSK_API *msk_delay_create(MskContainer *parent);

/* MSK World. */
MskContainer MSK_API *msk_world_create(gulong sample_rate, gsize block_size);
void MSK_API msk_world_destroy(MskContainer *world);
void MSK_API msk_world_prepare(MskContainer *container);
void MSK_API msk_world_run(MskContainer *container);
void MSK_API msk_message_note_on(MskWorld *world, short note, short velocity);
void MSK_API msk_message_note_off(MskWorld *world, short note, short velocity);

MskModule MSK_API *msk_factory_create_module(const char *name, MskContainer *parent);
MskContainer MSK_API *msk_factory_create_container(const char *name, MskContainer *parent);
MskContainer MSK_API *msk_load_world_from_file(const gchar *filename,
        MskModuleLoadCallback moduleload_callback, GError **error);
gboolean MSK_API msk_save_world_to_file(MskContainer *container, const gchar *filename,
        MskModuleSaveCallback modulesave_callback, GError **error);

// TODO: delete
void MSK_API msk_container_activate(MskContainer *self);
void MSK_API msk_container_deactivate(MskContainer *self);
