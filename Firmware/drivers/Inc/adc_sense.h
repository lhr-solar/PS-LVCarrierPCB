#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "ADC.h"
#include "stm32xx_hal.h"
#include "tmcs1126_lut.h"
#include "adc_lut.h"

adc_status_t adc_sense_init(void);
