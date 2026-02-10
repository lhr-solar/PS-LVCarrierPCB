#pragma once
#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"

typedef enum {
    STOP,
    START
} message_t;

void bq25756e_init(void);
void bq25756e_transmit(message_t message);
void i2c4_init(void);