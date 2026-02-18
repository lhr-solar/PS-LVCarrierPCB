#include "stm32xx_hal.h"
#include "pinDefs.h"
#include "common.h"
#include "lvEnable.h"
#include "statusLeds.h"

int main(){

    HAL_Init();
    SystemClock_Config();
    lvEnable_gpio_init();
    statusLeds_init();

    while(1){
        pin_state_t suppState = get_lvEnable_supp();
        pin_state_t powerSupplyState = get_lvEnable_powerSupply();

        if(suppState == ON){
            statusLeds_write(LSOM_HEARTBEAT_LED, ON);
        } else {
            statusLeds_write(LSOM_HEARTBEAT_LED, OFF);
        }

        if(powerSupplyState == ON){
            statusLeds_write(HEARTBEAT_LED, ON);
        } else {
            statusLeds_write(HEARTBEAT_LED, OFF);
        }

        HAL_Delay(1000);
    }

    return 0;
}
