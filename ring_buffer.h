#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct ring_buffer_t{
	size_t element_size; /* size of each data */
	size_t element_count; /* number of elements */
	size_t capacity; /* The max number of elements in ring buffer*/
	void *head;
	void *tail;
	void *begin;
	void *end;
} ring_buffer_t;

int ring_buffer_empty(ring_buffer_t *ring_buffer);
int ring_buffer_full(ring_buffer_t *ring_buffer);
void ring_buffer_init(ring_buffer_t *ring_buffer, size_t capacity, size_t element_size);
void ring_buffer_push(int *file_descriptor, ring_buffer_t *ring_buffer);
void ring_buffer_pop(ring_buffer_t *ring_buffer, void *item);

#endif