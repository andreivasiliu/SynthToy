/* My implementation of a JACK-like ringbuffer, because apparently jackdmp doesn't
 * have one. */

#include <glib.h>

#include "ringbuffer.h"


RingBuffer *ringbuffer_create(gsize size)
{
    RingBuffer *rb = g_new(RingBuffer, 1);

    rb->buffer = g_malloc((gsize) size);
    rb->size = size;
    rb->used = 0;
    rb->read_position = rb->buffer;
    rb->write_position = rb->buffer;
    
    return rb;
}

void ringbuffer_free(RingBuffer *rb)
{
    g_free(rb->buffer);
    g_free(rb);
}

gsize ringbuffer_read(RingBuffer *rb, char *dest, gsize count)
{
    char *src = rb->read_position;
    char *end = rb->buffer + rb->size;
    int i;
    
    if ( count > rb->used )
        count = rb->used;
    
    for ( i = 0; i < count; i++ )
    {
        *(dest++) = *(src++);
        
        if ( dest >= end )
            dest = rb->buffer;
    }
    
    rb->read_position += count;
    rb->used -= count;
    
    return count;
}

gsize ringbuffer_read_space(RingBuffer *rb)
{
    return rb->used;
}

gsize ringbuffer_write(RingBuffer *rb, char *src, gsize count)
{
    char *dest = rb->write_position;
    char *end = rb->buffer + rb->size;
    int i;
    
    if ( count > (rb->size - rb->used) )
        count = rb->size - rb->used;
    
    for ( i = 0; i < count; i++ )
    {
        *(dest++) = *(src++);
        
        if ( dest >= end )
            dest = rb->buffer;
    }
    
    rb->write_position = dest;
    rb->used += count;
    
    return count;
}

gsize ringbuffer_write_space(RingBuffer *rb)
{
    return rb->size - rb->used;
}

// Not thread safe.
void ringbuffer_reset(RingBuffer *rb)
{
    rb->read_position = rb->buffer;
    rb->write_position = rb->buffer;
    rb->used = 0;
}

