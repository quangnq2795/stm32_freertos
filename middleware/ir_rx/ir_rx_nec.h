#pragma once

#include <stddef.h>
#include <stdint.h>

#define IR_RX_NEC_WAVE_CAP  80U

typedef struct
{
  uint8_t address;
  uint8_t address_inv;
  uint8_t command;
  uint8_t command_inv;
  uint8_t is_repeat;
} ir_rx_nec_frame_t;

int ir_rx_nec_decode(const uint16_t *pulse_widths_us, size_t pulse_count,
                     ir_rx_nec_frame_t *out);
