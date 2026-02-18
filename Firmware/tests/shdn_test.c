#include "stm32xx_hal.h"
#include "statusLeds.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "common.h"

int main(){
    HAL_Init();
    SystemClock_Config();

    ltc4421_gpio_init();
    statusLeds_init();

    ltc4421_shdn_enable(OFF);
    HAL_Delay(5000);
    ltc4421_shdn_enable(ON);
    
    while(1){
        statusLeds_toggle(LSOM_HEARTBEAT_LED);
        HAL_Delay(1000);
    }

    return 0;
}
