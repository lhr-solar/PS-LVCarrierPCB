#pragma once

#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "commandLine.h"
#include "lvEnable.h"
#include "ltc4421.h"
#include "tasksConfig.h"
#include "ADC.h"
#include "adc_sense.h"
#include "timers.h"

int16_t adc_To_Hall(uint32_t adcCounts);
uint32_t adc_to_SuppVoltage(uint32_t adcCounts);