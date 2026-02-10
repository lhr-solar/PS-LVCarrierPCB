#pragma once

#include <stdint.h>

// TMCS1126C4x ADC lookup table
// Output units: milliamps (mA)
// Sensitivity: 100 mV/A
// Range: -2.3 A to +28.7 A

extern const int16_t adc_counts_to_ma_tmcs1126[4096];
