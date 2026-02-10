#!/usr/bin/env python3

ADC_BITS = 12
ADC_MAX_COUNTS = (1 << ADC_BITS) - 1
VREF_MV = 3300

SENSITIVITY_MV_PER_A = 100.0
CURRENT_MIN_A = -2.3

ARRAY_NAME = "adc_counts_to_ma_tmcs1126"
H_FILE = "tmcs1126_lut.h"
C_FILE = "tmcs1126_lut.c"

def generate_tmcs1126_lut():
    # ----- header -----
    with open(H_FILE, "w") as h:
        h.write("#pragma once\n\n")
        h.write("#include <stdint.h>\n\n")
        h.write("// TMCS1126C4x ADC lookup table\n")
        h.write("// Output units: milliamps (mA)\n")
        h.write("// Sensitivity: 100 mV/A\n")
        h.write("// Range: -2.3 A to +28.7 A\n\n")
        h.write(f"extern const int16_t {ARRAY_NAME}[{ADC_MAX_COUNTS + 1}];\n")

    # ----- source -----
    with open(C_FILE, "w") as c:
        c.write(f'#include "{H_FILE}"\n\n')
        c.write(f"const int16_t {ARRAY_NAME}[{ADC_MAX_COUNTS + 1}] = {{\n")

        for count in range(ADC_MAX_COUNTS + 1):
            mv = (count * VREF_MV) / ADC_MAX_COUNTS
            current_a = (mv / SENSITIVITY_MV_PER_A) + CURRENT_MIN_A
            current_ma = int(round(current_a * 1000.0))

            if count % 8 == 0:
                c.write("    ")

            c.write(f"{current_ma:6d}")

            if count != ADC_MAX_COUNTS:
                c.write(", ")

            if count % 8 == 7:
                c.write("\n")

        if ADC_MAX_COUNTS % 8 != 7:
            c.write("\n")

        c.write("};\n")

    print(f"Wrote {H_FILE} and {C_FILE}")

if __name__ == "__main__":
    generate_tmcs1126_lut()
