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
#include "faultBits.h"
#include "timers.h"


/**
 * @brief Converts adc counts to current measurement for the tmcs1126 Hall effect
 * 
 * @param adcCounts     ADC value between 0 to 4095
 * @return signed integer value for current measurement
 */
int16_t adc_To_Hall(uint32_t adcCounts);

/**
 * @brief Converts adc counts to voltage measurement
 * 
 * @param adcCounts     ADC value between 0 to 4095
 * @return unsigned integer value for voltage measurement
 */
uint32_t adc_to_SuppVoltage(uint32_t adcCounts);