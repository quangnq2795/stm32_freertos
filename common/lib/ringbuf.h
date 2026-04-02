#pragma once

#include <stddef.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Simple SPSC byte ring buffer:
 * - Producer: runs in UART RX callback (ISR context)
 * - Consumer: runs in application task
 *
 * Concurrency assumption:
 * - head is written only by producer, tail only by consumer.
 * - Only one producer and one consumer.
 *
 * Full condition: next(head) == tail (keeps one slot empty).
 * So max storable bytes = capacity - 1.
 *---------------------------------------------------------------------------*/

typedef struct
{
  uint8_t *storage;
  uint16_t capacity; /* raw storage size */
  volatile uint16_t head; /* next write index */
  volatile uint16_t tail; /* next read index */
} ringbuf_u8_t;

void ringbuf_init(ringbuf_u8_t *rb, uint8_t *storage, uint16_t capacity);

uint16_t ringbuf_capacity(const ringbuf_u8_t *rb);
uint16_t ringbuf_len(const ringbuf_u8_t *rb);
uint16_t ringbuf_space(const ringbuf_u8_t *rb);

size_t ringbuf_push_isr(ringbuf_u8_t *rb, const uint8_t *data, size_t len);
size_t ringbuf_pop(ringbuf_u8_t *rb, uint8_t *out, size_t len);

