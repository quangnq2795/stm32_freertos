#pragma once

#include <stdint.h>

#include "FreeRTOS.h"
#include "ir_rx_drv.h"
#include "ir_rx_nec.h"
#include "ir_tx_drv.h"

#define IR_OK           0
#define IR_ERR_PARAM   -1
#define IR_ERR_SEND    -2

typedef void (*ir_rx_frame_cb_t)(ir_rx_channel_id_t channel,
                                 const ir_rx_nec_frame_t *frame);

int ir_init(void);
void ir_uninit(void);
void ir_process(void);
int ir_register_rx_callback(ir_rx_frame_cb_t cb);

/* Post an IR TX request to IR task (SYS_NODE_IR) using NEC protocol. */
int ir_send_nec(ir_tx_channel_id_t channel,
                uint8_t address,
                uint8_t command,
                TickType_t timeout);

/* Post an IR TX repeat-code request (NEC protocol). */
int ir_send_nec_repeat(ir_tx_channel_id_t channel, TickType_t timeout);
