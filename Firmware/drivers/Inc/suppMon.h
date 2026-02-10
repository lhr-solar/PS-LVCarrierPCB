#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "ADC.h"
#include "stm32xx_hal.h"
#include "tmcs1126_lut.h"
#include "adcToMv_lut.h"

adc_status_t lv_carrier_adc_init(void);