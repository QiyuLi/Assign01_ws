#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__APPLE__)
  #include <malloc.h>
#endif
#include "ring_buffer.h"

// Initializes the buffer
void buffer_init(RingBuffer *ring_buffer, size_t capacity, size_t item_size)
{
  ring_buffer->buffer = malloc(capacity * item_size);

  // Error checking? To be optimized later
  if (ring_buffer->buffer == NULL) {
    return;
  }

  ring_buffer->buffer_end = (char *)ring_buffer->buffer + capacity * item_size;
  ring_buffer->capacity = capacity;
  ring_buffer->count = 0;
  ring_buffer->item_size = item_size;
  ring_buffer->head = ring_buffer->buffer;
  ring_buffer->tail = ring_buffer->buffer;
  return;
}

// Pushes value onto the queue
void push(int *file_descriptor, RingBuffer *ring_buffer)
{
  memcpy(ring_buffer->head, file_descriptor, ring_buffer->item_size);
  ring_buffer->head = (char *)ring_buffer->head + ring_buffer->item_size;
  if (ring_buffer->head == ring_buffer->buffer_end) {
    ring_buffer->head = ring_buffer->buffer;
  }
  ring_buffer->count++;
  return;
}


void pop(RingBuffer *ring_buffer, void *item)
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

int buffer_size(RingBuffer *ring_buffer)
{
  return ring_buffer->count;
}
