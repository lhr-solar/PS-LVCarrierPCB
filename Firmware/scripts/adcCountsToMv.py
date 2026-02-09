#!/usr/bin/env python3

ADC_BITS = 12
ADC_MAX_COUNTS = (1 << ADC_BITS) - 1  # 4095
VREF_MV = 3300                        # reference voltage in millivolts

ARRAY_NAME = "adc_counts_to_mv"
OUTPUT_FILE = "adcToMilliVolt.h.h"

def generate_c_lut_file(
    filename=OUTPUT_FILE,
    array_name=ARRAY_NAME,
    vref_mv=VREF_MV
):
    with open(filename, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write("// 12-bit ADC lookup table\n")
        f.write(f"// Vref = {vref_mv} mV\n")
        f.write(f"// ADC counts (0–{ADC_MAX_COUNTS}) -> millivolts\n\n")

        f.write(f"const uint16_t {array_name}[{ADC_MAX_COUNTS + 1}] = {{\n")

        for count in range(ADC_MAX_COUNTS + 1):
            mv = int(round((count * vref_mv) / ADC_MAX_COUNTS))

            if count % 8 == 0:
                f.write("    ")

            f.write(f"{mv:4d}")

            if count != ADC_MAX_COUNTS:
                f.write(", ")

            if count % 8 == 7:
                f.write("\n")

        if ADC_MAX_COUNTS % 8 != 7:
            f.write("\n")

        f.write("};\n")

    print(f"Wrote {filename}")

if __name__ == "__main__":
    generate_c_lut_file()
