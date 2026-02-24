#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "lvEnable.h"
#include "statusLeds.h"


int main(){

    HAL_Init();
    SystemClock_Config();
    statusLeds_init();

    while(1){
        // should never reach here
    }

}