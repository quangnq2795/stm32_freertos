#include "ringbuf.h"

static uint16_t ringbuf_next_index(const ringbuf_u8_t *rb, uint16_t i)
{
  return (uint16_t)((i + 1U) % rb->capacity);
}

void ringbuf_init(ringbuf_u8_t *rb, uint8_t *storage, uint16_t capacity)
{
  if (rb == 0 || storage == 0 || capacity < 2) {
    return;
  }

  rb->storage = storage;
  rb->capacity = capacity;
  rb->head = 0U;
  rb->tail = 0U;
}

uint16_t ringbuf_capacity(const ringbuf_u8_t *rb)
{
  return (rb == 0) ? 0U : rb->capacity;
}

uint16_t ringbuf_len(const ringbuf_u8_t *rb)
{
  if (rb == 0) {
    return 0U;
  }

  uint16_t head = rb->head;
  uint16_t tail = rb->tail;

  if (head >= tail) {
    return (uint16_t)(head - tail);
  }
  return (uint16_t)(rb->capacity - (tail - head));
}

uint16_t ringbuf_space(const ringbuf_u8_t *rb)
{
  if (rb == 0) {
    return 0U;
  }

  /* Keep one slot empty => usable space = capacity - 1 - len */
  uint16_t len = ringbuf_len(rb);
  if (rb->capacity <= 1U) {
    return 0U;
  }
  return (uint16_t)((rb->capacity - 1U) - len);
}

size_t ringbuf_push_isr(ringbuf_u8_t *rb, const uint8_t *data, size_t len)
{
  if (rb == 0 || data == 0 || len == 0) {
    return 0;
  }

  size_t pushed = 0;
  while (pushed < len) {
    uint16_t next = ringbuf_next_index(rb, rb->head);
    if (next == rb->tail) {
      /* Full */
      break;
    }
    rb->storage[rb->head] = data[pushed];
    rb->head = next;
    ++pushed;
  }
  return pushed;
}

size_t ringbuf_pop(ringbuf_u8_t *rb, uint8_t *out, size_t len)
{
  if (rb == 0 || out == 0 || len == 0) {
    return 0;
  }

  size_t popped = 0;
  while (popped < len) {
    if (rb->tail == rb->head) {
      /* Empty */
      break;
    }

    out[popped] = rb->storage[rb->tail];
    rb->tail = ringbuf_next_index(rb, rb->tail);
    ++popped;
  }
  return popped;
}

