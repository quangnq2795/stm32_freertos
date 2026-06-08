#include "ir_tx_nec.h"

#define NEC_LEADER_MARK_US   9000U
#define NEC_LEADER_SPACE_US  4500U
#define NEC_BIT_MARK_US      560U
#define NEC_ZERO_SPACE_US    560U
#define NEC_ONE_SPACE_US     1690U
#define NEC_REPEAT_MARK_US   9000U
#define NEC_REPEAT_SPACE_US  2250U
#define NEC_REPEAT_END_US    560U

static void nec_append_wave(ir_tx_wave_t *waves, size_t *wave_idx, size_t wave_cap,
                            uint16_t mark_us, uint16_t space_us)
{
  if (*wave_idx >= wave_cap) {
    return;
  }
  waves[*wave_idx].mark_us = mark_us;
  waves[*wave_idx].space_us = space_us;
  (*wave_idx)++;
}

size_t ir_tx_nec_encode(uint8_t address, uint8_t command, ir_tx_wave_t *waves,
                        size_t wave_cap)
{
  if (waves == NULL || wave_cap < 34U) {
    return 0U;
  }

  size_t wave_idx = 0U;
  nec_append_wave(waves, &wave_idx, wave_cap, NEC_LEADER_MARK_US,
                  NEC_LEADER_SPACE_US);

  uint32_t data = ((uint32_t)address << 24) |
                  ((uint32_t)(uint8_t)(~address) << 16) |
                  ((uint32_t)command << 8) | (uint32_t)(uint8_t)(~command);

  for (int bit = 31; bit >= 0; bit--) {
    uint16_t space_us = ((data >> (uint32_t)bit) & 1U) ? NEC_ONE_SPACE_US
                                                       : NEC_ZERO_SPACE_US;
    nec_append_wave(waves, &wave_idx, wave_cap, NEC_BIT_MARK_US, space_us);
  }

  if (wave_idx < wave_cap) {
    waves[wave_idx].mark_us = 560U;
    waves[wave_idx].space_us = 0U;
    wave_idx++;
  }

  return wave_idx;
}

size_t ir_tx_nec_encode_repeat(ir_tx_wave_t *waves, size_t wave_cap)
{
  if (waves == NULL || wave_cap < 3U) {
    return 0U;
  }

  size_t wave_idx = 0U;
  nec_append_wave(waves, &wave_idx, wave_cap, NEC_REPEAT_MARK_US,
                  NEC_REPEAT_SPACE_US);
  if (wave_idx < wave_cap) {
    waves[wave_idx].mark_us = NEC_REPEAT_END_US;
    waves[wave_idx].space_us = 0U;
    wave_idx++;
  }
  return wave_idx;
}
