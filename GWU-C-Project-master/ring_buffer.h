#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct RingBuffer{
  void *head;
  void *tail;
  void *buffer;
  void *buffer_end;
  size_t capacity;
  size_t count;
  size_t item_size;
} RingBuffer;

void buffer_init(RingBuffer *ring_buffer, size_t capacity, size_t item_size);
void push(int *file_descriptor, RingBuffer *ring_buffer);
void pop(RingBuffer *ring_buffer, void *item);
int buffer_size(RingBuffer *ring_buffer);

#endif