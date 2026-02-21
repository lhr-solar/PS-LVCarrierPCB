#!/usr/bin/env python3

ADC_BITS = 12
ADC_MAX_COUNTS = (1 << ADC_BITS) - 1  # 4095
VREF_MV = 3300

ARRAY_NAME = "adc_counts_to_mv"
H_FILE = "adc_lut.h"
C_FILE = "adc_lut.c"

def generate_adc_mv_lut():
    # ----- header -----
    with open(H_FILE, "w") as h:
        h.write("#pragma once\n\n")
        h.write("#include <stdint.h>\n\n")
        h.write("// 12-bit ADC lookup table\n")
        h.write(f"// Vref = {VREF_MV} mV\n\n")
        h.write(f"extern const uint16_t {ARRAY_NAME}[{ADC_MAX_COUNTS + 1}];\n")

    # ----- source -----
    with open(C_FILE, "w") as c:
        c.write(f'#include "{H_FILE}"\n\n')
        c.write(f"const uint16_t {ARRAY_NAME}[{ADC_MAX_COUNTS + 1}] = {{\n")

        for count in range(ADC_MAX_COUNTS + 1):
            mv = int(round((count * VREF_MV) / ADC_MAX_COUNTS))

            if count % 8 == 0:
                c.write("    ")

            c.write(f"{mv:4d}")

            if count != ADC_MAX_COUNTS:
                c.write(", ")

            if count % 8 == 7:
                c.write("\n")

        if ADC_MAX_COUNTS % 8 != 7:
            c.write("\n")

        c.write("};\n")

    print(f"Wrote {H_FILE} and {C_FILE}")

if __name__ == "__main__":
    generate_adc_mv_lut()
