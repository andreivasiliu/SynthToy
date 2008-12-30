#include <glib/gtypes.h>

#define MSK_AUDIO_DATA      1
#define MSK_FLOAT_PROPERTY  2
#define MSK_INT_PROPERTY  2

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


struct _MskWorld
{
    guint sample_rate;
    gsize block_size;
    
    MskContainer *root;
    
    GList *instruments;
    
    GMutex *lock_for_model;
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
    
    MskProcessCallback process;
    MskActivateCallback activate;
    MskDeactivateCallback deactivate;
    MskGlobalActivateCallback global_activate;
    MskGlobalDeactivateCallback global_deactivate;
    
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
};

struct _MskProperty
{
    gchar *name;
    guint type;
    gpointer value;
    MskModule *owner;
};


/*** Functions ***/
void msk_module_activate(MskModule *mod);
void msk_module_deactivate(MskModule *mod);

void msk_module_set_float_property(MskModule *mod, gchar *name, gfloat value);

gconstpointer msk_module_get_property_buffer(MskModule *mod, gchar *name);
gconstpointer msk_module_get_input_buffer(MskModule *mod, gchar *name);
gpointer      msk_module_get_output_buffer(MskModule *mod, gchar *name);

void msk_connect_ports(MskModule *left, gchar *left_port_name,
                       MskModule *right, gchar *right_port_name);

MskContainer *msk_container_create(MskContainer *parent);
MskContainer *msk_instrument_create(MskContainer *parent);

MskModule *msk_input_create_with_name(MskContainer *parent, gchar *name, guint type);
MskModule *msk_output_create_with_name(MskContainer *parent, gchar *name, guint type);

MskModule *msk_input_create(MskContainer *parent);
MskModule *msk_output_create(MskContainer *parent);
MskModule *msk_voice_create(MskContainer *parent);
MskModule *msk_constant_create(MskContainer *parent);
MskModule *msk_autoconstant_create(MskContainer *parent);
MskModule *msk_oscillator_create(MskContainer *parent);
MskModule *msk_pitchtofrequency_create(MskContainer *parent);
MskModule *msk_addmul_create(MskContainer *parent);
MskModule *msk_add_create(MskContainer *parent);
MskModule *msk_mul_create(MskContainer *parent);
MskModule *msk_voiceactive_create(MskContainer *parent);
MskModule *msk_voicepitch_create(MskContainer *parent);
MskModule *msk_voicevelocity_create(MskContainer *parent);


MskContainer *msk_world_create(gulong sample_rate, gsize block_size);
void msk_world_prepare(MskContainer *container);
void msk_world_run(MskContainer *container);
void msk_message_note_on(MskWorld *world, short note, short velocity);
void msk_message_note_off(MskWorld *world, short note, short velocity);


// TODO: delete
void msk_create_buffers_on_module(MskModule *module);
void msk_destroy_buffers_on_module(MskModule *module);
void msk_container_activate(MskContainer *self);
void msk_container_deactivate(MskContainer *self);
