#pragma once

#include <stddef.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 * SPSC fixed-size slot queue (one producer, one consumer):
 *
 * - Storage is a caller-owned array of `depth` elements, each `elem_size` bytes.
 * - Producer (typically ISR): slot_queue_acquire → write slot → slot_queue_release
 * - Consumer (typically task): slot_queue_try_pop
 *
 * head/tail/count: producer updates head+count on release; consumer updates tail+count on pop.
 *---------------------------------------------------------------------------*/

#define SLOT_QUEUE_OK      0
#define SLOT_QUEUE_EMPTY  -1
#define SLOT_QUEUE_FULL   -2
#define SLOT_QUEUE_PARAM  -3

typedef struct
{
  uint8_t *storage;
  uint16_t elem_size;
  uint8_t depth;
  volatile uint8_t head;
  volatile uint8_t tail;
  volatile uint8_t count;
} slot_queue_t;

void slot_queue_init(slot_queue_t *q, void *storage, uint16_t elem_size,
                     uint8_t depth);
void slot_queue_reset(slot_queue_t *q);

uint8_t slot_queue_count(const slot_queue_t *q);
uint8_t slot_queue_space(const slot_queue_t *q);
uint8_t slot_queue_is_empty(const slot_queue_t *q);
uint8_t slot_queue_is_full(const slot_queue_t *q);

/* Copy-in push (task context or when source is separate). */
int slot_queue_try_push(slot_queue_t *q, const void *elem);

/* In-place push: acquire writable slot, fill it, then release. */
void *slot_queue_acquire(slot_queue_t *q);
void slot_queue_release(slot_queue_t *q);

/* Copy-out pop (task context). */
int slot_queue_try_pop(slot_queue_t *q, void *out);
