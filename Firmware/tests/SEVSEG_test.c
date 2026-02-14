#include "ShiftRegister.h"
#include "stm32xx_hal.h"

int main(void) {
  HAL_Init();
  SystemClock_Config();
  ShiftRegister_Init();

  uint8_t d0, d1, d2;

  while (1) {
    for (uint16_t val = 0; val <= MAX_NUM; val++) {
      d0 = (val / 100) % 10;
      d1 = (val / 10) % 10;
      d2 = val % 10;

      DisplayData_SevenSeg(d0, d1, d2);

      HAL_Delay(100);
    }
  }
}
