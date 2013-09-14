/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2013 by Qiyu Li.
 * Author: Qiyu Li, johnnyli@gwu.edu, 2013
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct ring_buffer_t {
    /* element related number */
    size_t element_size; /* size of each data */
    size_t element_count; /* number of elements */
    size_t element_capacity; /* The max number of elements in ring buffer*/

    /* ring buffer pointers */
    void *head; /* The address of first data */
    void *tail; /* The address of last data */
    void *begin; /* The first address of the memory space */
    void *end; /* The last address of the memory space */
} ring_buffer_t;

/*
 * Initial ring buffer, with defined element capacity and element size
 */
void ring_buffer_init(ring_buffer_t *ring_buffer, size_t element_capacity, size_t element_size);

/*
 * Check if the ring buffer is empty,
 * if it is empty, return 0
 */
int ring_buffer_is_empty(ring_buffer_t *ring_buffer);

/*
 * Check if the ring buffer is full,
 * if it is full, return 0
 */
int ring_buffer_is_full(ring_buffer_t *ring_buffer);

/*
 * Push a data in to the ring buffer
 */
void ring_buffer_push(void *data, ring_buffer_t *ring_buffer);

/*
 * Pop a data from the ring buffer
 */
void ring_buffer_pop(ring_buffer_t *ring_buffer, void *data);

#endif