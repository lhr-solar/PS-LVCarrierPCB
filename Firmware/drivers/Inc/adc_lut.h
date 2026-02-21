#pragma once

#include <stdint.h>

// 12-bit ADC lookup table
// Vref = 3300 mV

extern const uint16_t adc_counts_to_mv[4096];
