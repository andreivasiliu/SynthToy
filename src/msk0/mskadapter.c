#include <glib.h>

#include "msk0.h"
#include "mskinternal.h"


static void convert_controldata_to_audiodata(void *cdata, void *adata, int start, int frames);

struct
{
    guint source_port_type;
    guint destination_port_type;

    MskAdapterCallback adapter_callback;
} adapters[] =
{
    { MSK_CONTROL_DATA, MSK_AUDIO_DATA, &convert_controldata_to_audiodata },
    { 0, 0, NULL }
};

static void convert_controldata_to_audiodata(void *cdata, void *adata, int start, int frames)
{
    int i;

    for ( i = start; i < start + frames; i++ )
        ((float*) adata)[i] = ((float*) cdata)[0];
}

MskAdapterCallback msk_get_adapter(guint source_port_type, guint destination_port_type)
{
    int i;

    for ( i = 0; adapters[i].adapter_callback; i++ )
        if ( adapters[i].source_port_type == source_port_type &&
             adapters[i].destination_port_type == destination_port_type )
            return adapters[i].adapter_callback;

    return NULL;
}
