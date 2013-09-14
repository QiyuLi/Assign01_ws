#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ring_buffer.h>

/*
 * Initialize the ring buffer
 */
void ring_buffer_init(ring_buffer_t *ring_buffer, size_t capacity, size_t item_size)
{
	ring_buffer->begin = malloc( (capacity + 1) * item_size);
	ring_buffer->end = (char *)ring_buffer->begin + capacity * item_size;
	ring_buffer->capacity = capacity;
	ring_buffer->count = 0;
	ring_buffer->item_size = item_size;
	ring_buffer->head = ring_buffer->begin;
	ring_buffer->tail = ring_buffer->begin;
	return;
}

int ring_buffer_empty(ring_buffer_t *ring_buffer)
{
	if(ring_buffer->count == 0)
		return 0;
	return -1;
}

int ring_buffer_full(ring_buffer_t *ring_buffer)
{
	if(ring_buffer->count == ring_buffer->capacity)
		return 0;
	return -1;
}

// Pushes value onto the queue
void push(int *file_descriptor, ring_buffer_t *ring_buffer)
{
  memcpy(ring_buffer->head, file_descriptor, ring_buffer->item_size);
  ring_buffer->head = (char *)ring_buffer->head + ring_buffer->item_size;
  if (ring_buffer->head == ring_buffer->buffer_end) {
    ring_buffer->head = ring_buffer->buffer;
  }
  ring_buffer->count++;
  return;
}


void pop(ring_buffer_t *ring_buffer, void *item)
{
  if (ring_buffer->count == 0) {
    return;
  }
  memcpy(item, ring_buffer->tail, ring_buffer->item_size);
  ring_buffer->tail = (char *)ring_buffer->tail + ring_buffer->item_size;
  
  if (ring_buffer->tail == ring_buffer->buffer_end) {
    ring_buffer->tail = ring_buffer->buffer;
  }
  ring_buffer->count--;
}
