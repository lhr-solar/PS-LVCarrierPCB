#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "statusLeds.h"

int main(){

    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    for(uint8_t i = 0; i < NUM_STATUS_LEDs; i++){
        statusLeds_write(i, ON);
    }
    HAL_Delay(100);

    for(uint8_t i = 0; i < NUM_STATUS_LEDs; i++){
        statusLeds_write(i, OFF);
    }
    HAL_Delay(100);

    while(1){
        for(uint8_t i = 0; i < NUM_STATUS_LEDs; i++){
            statusLeds_toggle(i);
            HAL_Delay(1000);

        }
        
    }

    return 0;
}