#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bsp_ir_rx_cfg.h"
#include "ir_rx_drv.h"
#include "ir_rx_nec.h"

#define IR_RX_BURST_QUEUE_DEPTH  3U

#define IR_RX_OPCODE_BURST_READY  0U

#define IR_RX_OK           0
#define IR_RX_ERR_EMPTY   -1
#define IR_RX_ERR_PARAM   -2
#define IR_RX_ERR_DECODE  -3

typedef struct
{
  uint16_t count;
  uint16_t pulses[BSP_IR_RX_MAX_PULSE_RING_CAP];
} ir_rx_burst_t;

void ir_rx_init(ir_rx_channel_id_t channel);
void ir_rx_init_all(void);

int ir_rx_try_read_nec(ir_rx_channel_id_t channel, ir_rx_nec_frame_t *out);
