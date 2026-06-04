#include "slot_queue.h"

#include <string.h>

static uint8_t slot_queue_next_index(uint8_t idx, uint8_t depth)
{
  return (uint8_t)((idx + 1U) % depth);
}

static uint8_t *slot_queue_elem_ptr(const slot_queue_t *q, uint8_t index)
{
  return q->storage + ((size_t)index * (size_t)q->elem_size);
}

void slot_queue_init(slot_queue_t *q, void *storage, uint16_t elem_size,
                     uint8_t depth)
{
  if (q == NULL || storage == NULL || elem_size == 0U || depth == 0U) {
    return;
  }

  q->storage = (uint8_t *)storage;
  q->elem_size = elem_size;
  q->depth = depth;
  q->head = 0U;
  q->tail = 0U;
  q->count = 0U;
}

void slot_queue_reset(slot_queue_t *q)
{
  if (q == NULL) {
    return;
  }

  q->head = 0U;
  q->tail = 0U;
  q->count = 0U;
}

uint8_t slot_queue_count(const slot_queue_t *q)
{
  if (q == NULL) {
    return 0U;
  }

  return q->count;
}

uint8_t slot_queue_space(const slot_queue_t *q)
{
  if (q == NULL || q->depth == 0U) {
    return 0U;
  }

  return (uint8_t)(q->depth - q->count);
}

uint8_t slot_queue_is_empty(const slot_queue_t *q)
{
  if (q == NULL) {
    return 1U;
  }

  return (q->count == 0U) ? 1U : 0U;
}

uint8_t slot_queue_is_full(const slot_queue_t *q)
{
  if (q == NULL || q->depth == 0U) {
    return 1U;
  }

  return (q->count >= q->depth) ? 1U : 0U;
}

void *slot_queue_acquire(slot_queue_t *q)
{
  if (q == NULL || q->storage == NULL || q->depth == 0U) {
    return NULL;
  }

  if (slot_queue_is_full(q) != 0U) {
    return NULL;
  }

  return slot_queue_elem_ptr(q, q->head);
}

void slot_queue_release(slot_queue_t *q)
{
  if (q == NULL || q->depth == 0U || slot_queue_is_full(q) != 0U) {
    return;
  }

  q->head = slot_queue_next_index(q->head, q->depth);
  q->count++;
}

int slot_queue_try_push(slot_queue_t *q, const void *elem)
{
  void *slot;

  if (q == NULL || elem == NULL) {
    return SLOT_QUEUE_PARAM;
  }

  slot = slot_queue_acquire(q);
  if (slot == NULL) {
    return SLOT_QUEUE_FULL;
  }

  (void)memcpy(slot, elem, (size_t)q->elem_size);
  slot_queue_release(q);
  return SLOT_QUEUE_OK;
}

int slot_queue_try_pop(slot_queue_t *q, void *out)
{
  if (q == NULL || out == NULL) {
    return SLOT_QUEUE_PARAM;
  }

  if (slot_queue_is_empty(q) != 0U) {
    return SLOT_QUEUE_EMPTY;
  }

  (void)memcpy(out, slot_queue_elem_ptr(q, q->tail), (size_t)q->elem_size);
  q->tail = slot_queue_next_index(q->tail, q->depth);
  q->count--;

  return SLOT_QUEUE_OK;
}
