#include <stddef.h>

#include "h_soft_timer.h"

#include "h_timer.h"

#define H_SOFT_TIMER_MAX_ENTRIES  16U
#define H_SOFT_TIMER_NONE_DEADLINE  0xFFFFFFFFU

typedef struct
{
  uint8_t active;
  uint32_t deadline;
  h_soft_timer_callback_t cb;
  void *ctx;
} h_soft_timer_entry_t;

static h_soft_timer_entry_t s_entries[H_SOFT_TIMER_MAX_ENTRIES];
static uint32_t s_next_deadline = H_SOFT_TIMER_NONE_DEADLINE;

static uint8_t h_soft_timer_deadline_due(uint32_t deadline, uint32_t now)
{
  return (uint32_t)(now - deadline) < 0x80000000U ? 1U : 0U;
}

static uint32_t h_soft_timer_find_earliest(void)
{
  uint32_t best = H_SOFT_TIMER_NONE_DEADLINE;
  uint32_t i;

  for (i = 0U; i < H_SOFT_TIMER_MAX_ENTRIES; i++) {
    if (s_entries[i].active == 0U) {
      continue;
    }
    if (best == H_SOFT_TIMER_NONE_DEADLINE || s_entries[i].deadline < best) {
      best = s_entries[i].deadline;
    }
  }
  return best;
}

static void h_soft_timer_reschedule(void)
{
  uint32_t now = h_timer_tick_us();
  uint32_t earliest = h_soft_timer_find_earliest();

  s_next_deadline = earliest;
  if (earliest == H_SOFT_TIMER_NONE_DEADLINE) {
    h_timer_stop_compare();
    return;
  }

  if (h_soft_timer_deadline_due(earliest, now)) {
    h_timer_schedule(now + 1U);
  } else {
    h_timer_schedule(earliest);
  }
}

static void h_soft_timer_expire_hook(void)
{
  uint32_t now = h_timer_tick_us();
  uint32_t i;
  uint8_t fired = 0U;

  for (i = 0U; i < H_SOFT_TIMER_MAX_ENTRIES; i++) {
    if (s_entries[i].active == 0U) {
      continue;
    }
    if (!h_soft_timer_deadline_due(s_entries[i].deadline, now)) {
      continue;
    }

    h_soft_timer_callback_t cb = s_entries[i].cb;
    void *ctx = s_entries[i].ctx;
    s_entries[i].active = 0U;
    fired = 1U;

    if (cb != NULL) {
      cb(ctx);
    }
  }

  if (fired != 0U) {
    h_soft_timer_reschedule();
  }
}

void h_soft_timer_init(void)
{
  uint32_t i;

  h_timer_init();
  h_timer_set_expire_hook(h_soft_timer_expire_hook);

  for (i = 0U; i < H_SOFT_TIMER_MAX_ENTRIES; i++) {
    s_entries[i].active = 0U;
    s_entries[i].deadline = 0U;
    s_entries[i].cb = NULL;
    s_entries[i].ctx = NULL;
  }
  s_next_deadline = H_SOFT_TIMER_NONE_DEADLINE;
}

uint32_t h_soft_timer_tick(void)
{
  return h_timer_tick_us();
}

int h_soft_timer_register(uint32_t delay_us, h_soft_timer_callback_t cb, void *ctx)
{
  uint32_t i;
  uint32_t now;
  uint32_t deadline;

  if (cb == NULL || delay_us == 0U) {
    return H_SOFT_TIMER_INVALID_ID;
  }

  for (i = 0U; i < H_SOFT_TIMER_MAX_ENTRIES; i++) {
    if (s_entries[i].active != 0U) {
      continue;
    }

    now = h_timer_tick_us();
    deadline = now + delay_us;

    s_entries[i].active = 1U;
    s_entries[i].deadline = deadline;
    s_entries[i].cb = cb;
    s_entries[i].ctx = ctx;

    if (s_next_deadline == H_SOFT_TIMER_NONE_DEADLINE ||
        deadline < s_next_deadline) {
      h_soft_timer_reschedule();
    }
    return (int)i;
  }

  return H_SOFT_TIMER_INVALID_ID;
}

void h_soft_timer_unregister(int id)
{
  if (id < 0 || (uint32_t)id >= H_SOFT_TIMER_MAX_ENTRIES) {
    return;
  }

  s_entries[(uint32_t)id].active = 0U;
  s_entries[(uint32_t)id].cb = NULL;
  s_entries[(uint32_t)id].ctx = NULL;
  h_soft_timer_reschedule();
}
