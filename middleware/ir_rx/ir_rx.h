#pragma once

#include <stdint.h>

#include "ir_rx_drv.h"
#include "ir_rx_nec.h"

typedef void (*ir_rx_frame_cb_t)(ir_rx_channel_id_t channel,
                                 const ir_rx_nec_frame_t *frame, void *ctx);

void ir_rx_init(ir_rx_channel_id_t channel);
void ir_rx_init_all(void);

void ir_rx_register_frame_callback(ir_rx_channel_id_t channel,
                                   ir_rx_frame_cb_t cb, void *ctx);
