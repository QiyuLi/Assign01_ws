/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2013 by Qiyu Li.
 * Author: Qiyu Li, johnnyli@gwu.edu, 2013
 */

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <ring_buffer.h>

/*
 * Initialize the ring buffer
 */
void ring_buffer_init(ring_buffer_t *ring_buffer, size_t element_size, size_t element_capacity)
{
    /* Create a continues memory space */
    ring_buffer->begin = malloc( (element_capacity + 1) * element_size);
    ring_buffer->end = (char *)ring_buffer->begin + element_capacity * element_size;
    ring_buffer->element_capacity = element_capacity;
    ring_buffer->element_count = 0;
    ring_buffer->element_size = element_size;
    ring_buffer->head = ring_buffer->begin;
    ring_buffer->tail = ring_buffer->begin;
    return;
}

int ring_buffer_is_empty(ring_buffer_t *ring_buffer)
{
    if(ring_buffer->element_count == 0)
        return 0;
    return -1;
}

int ring_buffer_is_full(ring_buffer_t *ring_buffer)
{
    if(ring_buffer->element_count == ring_buffer->element_capacity)
        return 0;
    return -1;
}

void ring_buffer_push(void *data, ring_buffer_t *ring_buffer)
{
    if(ring_buffer_is_full(ring_buffer) == 0) {
        return;
    }

    memcpy(ring_buffer->head, data, ring_buffer->element_size);
    ring_buffer->head = (char *)ring_buffer->head + ring_buffer->element_size;
    if (ring_buffer->head == ring_buffer->end) {
        ring_buffer->head = ring_buffer->begin;
    }
    ring_buffer->element_count++;
    return;
}


void ring_buffer_pop(ring_buffer_t *ring_buffer, void *data)
{
    if (ring_buffer_is_empty(ring_buffer) == 0) {
        return;
    }

    memcpy(data, ring_buffer->tail, ring_buffer->element_size);
    ring_buffer->tail = (char *)ring_buffer->tail + ring_buffer->element_size;

    if (ring_buffer->tail == ring_buffer->end) {
        ring_buffer->tail = ring_buffer->begin;
    }
    ring_buffer->element_count--;
}
