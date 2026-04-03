#pragma once

#include <stddef.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 * Simple SPSC byte ring buffer (one producer, one consumer):
 * - This project uses it for UART RX/TX.
 * - The "push_isr" and "push" APIs are provided for readability; in this
 *   implementation they behave the same (no extra locking).
 *
 * head: producer only; tail: consumer only.
 * Full: next(head) == tail → max stored = capacity - 1.
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

size_t ringbuf_push(ringbuf_u8_t *rb, const uint8_t *data, size_t len);
size_t ringbuf_pop(ringbuf_u8_t *rb, uint8_t *out, size_t len);

