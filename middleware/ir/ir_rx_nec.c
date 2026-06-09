#include "ir_rx_nec.h"

#include <string.h>

#define NEC_LEADER_MARK_US   9000U
#define NEC_LEADER_SPACE_US  4500U
#define NEC_BIT_MARK_US      560U
#define NEC_ZERO_SPACE_US    560U
#define NEC_ONE_SPACE_US     1690U
#define NEC_REPEAT_MARK_US   9000U
#define NEC_REPEAT_SPACE_US  2250U

#define NEC_TOL_PCT  50U

static uint8_t nec_pulse_matches(uint16_t value_us, uint16_t target_us)
{
  uint32_t lo = (uint32_t)target_us * (100U - NEC_TOL_PCT) / 100U;
  uint32_t hi = (uint32_t)target_us * (100U + NEC_TOL_PCT) / 100U;
  return (value_us >= lo && value_us <= hi) ? 1U : 0U;
}

static int nec_decode_data_bits(const uint16_t *pulse_widths_us, size_t start_idx,
                                size_t pulse_count, uint32_t *data_out)
{
  uint32_t data = 0U;

  for (size_t bit = 0U; bit < 32U; bit++) {
    size_t mark_idx = start_idx + (bit * 2U);
    size_t space_idx = mark_idx + 1U;
    if (space_idx >= pulse_count) {
      return -1;
    }
    if (!nec_pulse_matches(pulse_widths_us[mark_idx], NEC_BIT_MARK_US)) {
      return -1;
    }
    if (nec_pulse_matches(pulse_widths_us[space_idx], NEC_ONE_SPACE_US)) {
      data = (data << 1) | 1U;
    } else if (nec_pulse_matches(pulse_widths_us[space_idx], NEC_ZERO_SPACE_US)) {
      data = (data << 1);
    } else {
      return -1;
    }
  }

  *data_out = data;
  return 0;
}

int ir_rx_nec_decode(const uint16_t *pulse_widths_us, size_t pulse_count,
                     ir_rx_nec_frame_t *out)
{
  if (pulse_widths_us == NULL || out == NULL || pulse_count < 3U) {
    return -1;
  }

  (void)memset(out, 0, sizeof(*out));

  if (nec_pulse_matches(pulse_widths_us[0], NEC_REPEAT_MARK_US) &&
      pulse_count >= 3U &&
      nec_pulse_matches(pulse_widths_us[1], NEC_REPEAT_SPACE_US)) {
    out->is_repeat = 1U;
    return 0;
  }

  if (!nec_pulse_matches(pulse_widths_us[0], NEC_LEADER_MARK_US) ||
      !nec_pulse_matches(pulse_widths_us[1], NEC_LEADER_SPACE_US)) {
    return -1;
  }

  uint32_t data;
  if (nec_decode_data_bits(pulse_widths_us, 2U, pulse_count, &data) != 0) {
    return -1;
  }

  out->address = (uint8_t)(data >> 24);
  out->address_inv = (uint8_t)(data >> 16);
  out->command = (uint8_t)(data >> 8);
  out->command_inv = (uint8_t)data;

  if ((uint8_t)(out->address ^ out->address_inv) != 0xFFU ||
      (uint8_t)(out->command ^ out->command_inv) != 0xFFU) {
    return -1;
  }

  return 0;
}
