#pragma once

#include <stddef.h>
#include <stdint.h>

#include "ir_tx_drv.h"

#define IR_TX_NEC_WAVE_CAP  80U

size_t ir_tx_nec_encode(uint8_t address, uint8_t command, ir_tx_wave_t *waves,
                        size_t wave_cap);
size_t ir_tx_nec_encode_repeat(ir_tx_wave_t *waves, size_t wave_cap);
