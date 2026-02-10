#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "statusLeds.h"
#include "pinDefs.h"
#include "ltc4421.h"
#include "lvEnable.h"

int main(){
    HAL_Init(); // reset all peripherals

    SystemClock_Config(); // configure clock
    
    statusLeds_init();
    ltc4421_gpio_init();
    lvEnable_gpio_init();

    while(1){
        
    }

}