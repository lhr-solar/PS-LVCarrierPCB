#include "stm32xx_hal.h"
#include "statusLeds.h"
#include "stdint.h"
#include "shiftRegister.h"

int main(void) {
  HAL_Init();
  SystemClock_Config();

  shiftRegister_init();
  statusLeds_init();

  // uint8_t d0, d1, d2;

  display_sevenSeg(1, 3, 6);

  while (1) {
    // for (uint16_t val = 0; val <= MAX_NUM; val++) {
    //   d0 = (val / 100) % 10;
    //   d1 = (val / 10) % 10;
    //   d2 = val % 10;

    //   display_sevenSeg(d0, d1, d2);

    //   HAL_Delay(1000);
    //   statusLeds_toggle(LSOM_HEARTBEAT_LED);
    // }
     HAL_Delay(1000);
    statusLeds_toggle(LSOM_HEARTBEAT_LED);
  }
}
