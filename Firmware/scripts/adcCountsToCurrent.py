#!/usr/bin/env python3

ADC_BITS = 12
ADC_MAX_COUNTS = (1 << ADC_BITS) - 1   # 4095
VREF_MV = 3300                         # ADC reference voltage

# TMCS1126C4x characteristics
SENSITIVITY_MV_PER_A = 100.0
CURRENT_MIN_A = -2.3
CURRENT_MAX_A = 28.7

ARRAY_NAME = "adc_counts_to_ma_tmcs1126"
OUTPUT_FILE = "tmcs1126_lut.h"

def generate_tmcs1126_lut(
    filename=OUTPUT_FILE,
    array_name=ARRAY_NAME
):
    with open(filename, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("// TMCS1126C4x ADC lookup table\n")
        f.write("// Sensitivity: 100 mV/A\n")
        f.write("// Current range: -2.3 A to +28.7 A\n")
        f.write("// Output: milliamps (mA)\n")
        f.write("// ADC: 12-bit, Vref = 3300 mV\n\n")

        f.write(f"const int16_t {array_name}[{ADC_MAX_COUNTS + 1}] = {{\n")

        for count in range(ADC_MAX_COUNTS + 1):
            # ADC count -> millivolts
            mv = (count * VREF_MV) / ADC_MAX_COUNTS

            # millivolts -> current (A)
            current_a = (mv / SENSITIVITY_MV_PER_A) + CURRENT_MIN_A

            # convert to milliamps
            current_ma = int(round(current_a * 1000.0))

            if count % 8 == 0:
                f.write("    ")

            f.write(f"{current_ma:6d}")

            if count != ADC_MAX_COUNTS:
                f.write(", ")

            if count % 8 == 7:
                f.write("\n")

        if ADC_MAX_COUNTS % 8 != 7:
            f.write("\n")

        f.write("};\n")

    print(f"Wrote {filename}")

if __name__ == "__main__":
    generate_tmcs1126_lut()
