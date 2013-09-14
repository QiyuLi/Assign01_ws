#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ring_buffer.h>

/*
 * Initialize the ring buffer
 */
void ring_buffer_init(ring_buffer_t *ring_buffer, size_t capacity, size_t element_size)
{
	ring_buffer->begin = malloc( (capacity + 1) * element_size);
	ring_buffer->end = (char *)ring_buffer->begin + capacity * element_size;
	ring_buffer->capacity = capacity;
	ring_buffer->element_count = 0;
	ring_buffer->element_size = element_size;
	ring_buffer->head = ring_buffer->begin;
	ring_buffer->tail = ring_buffer->begin;
	return;
}

int ring_buffer_empty(ring_buffer_t *ring_buffer)
{
	if(ring_buffer->element_count == 0)
		return 0;
	return -1;
}

int ring_buffer_full(ring_buffer_t *ring_buffer)
{
	printf("Buffer queue number: %d\n", ring_buffer->element_count);
	if(ring_buffer->element_count == ring_buffer->capacity)
		return 0;
	return -1;
}

// Pushes value onto the queue
void ring_buffer_push(int *file_descriptor, ring_buffer_t *ring_buffer)
{
  memcpy(ring_buffer->head, file_descriptor, ring_buffer->element_size);
  ring_buffer->head = (char *)ring_buffer->head + ring_buffer->element_size;
  if (ring_buffer->head == ring_buffer->end) {
    ring_buffer->head = ring_buffer->begin;
  }
  ring_buffer->element_count++;
  return;
}


void ring_buffer_pop(ring_buffer_t *ring_buffer, void *item)
{
  if (ring_buffer->element_count == 0) {
    return;
  }
  memcpy(item, ring_buffer->tail, ring_buffer->element_size);
  ring_buffer->tail = (char *)ring_buffer->tail + ring_buffer->element_size;
  
  if (ring_buffer->tail == ring_buffer->end) {
    ring_buffer->tail = ring_buffer->begin;
  }
  ring_buffer->element_count--;
}
