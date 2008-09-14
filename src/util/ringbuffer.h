
#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

// TODO: the above conflicts with JACK's header.

#include <glib/gtypes.h>

typedef struct _RingBuffer
{
    gsize size;
    gsize used;

    char *buffer;

    char *read_position;
    char *write_position;
}
RingBuffer;


RingBuffer *ringbuffer_create(gsize size);
void ringbuffer_free(RingBuffer *rb);
gsize ringbuffer_read(RingBuffer *rb, char *dest, gsize count);
gsize ringbuffer_read_space(RingBuffer *rb);
gsize ringbuffer_write(RingBuffer *rb, char *src, gsize count);
gsize ringbuffer_write_space(RingBuffer *rb);
void ringbuffer_reset(RingBuffer *rb);

#endif /* _RINGBUFFER_H */
