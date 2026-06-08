#pragma once

#include <stdint.h>

#include "ir_tx_drv.h"

void ir_tx_init(ir_tx_channel_id_t channel);
void ir_tx_init_all(void);

int ir_tx_send_nec(ir_tx_channel_id_t channel, uint8_t address, uint8_t command);
int ir_tx_send_nec_repeat(ir_tx_channel_id_t channel);
