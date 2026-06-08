#pragma once

#include "ir_rx_drv.h"
#include "ir_rx_nec.h"

#define IR_OK           0
#define IR_ERR_PARAM   -1

typedef void (*ir_rx_frame_cb_t)(ir_rx_channel_id_t channel,
                                 const ir_rx_nec_frame_t *frame);

int ir_init(void);
void ir_uninit(void);
void ir_process(void);
int ir_register_rx_callback(ir_rx_frame_cb_t cb);
