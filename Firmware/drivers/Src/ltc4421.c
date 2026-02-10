#include "ltc4421.h"


void ltc4421_gpio_init(void){
    
}

void ltc4421_shdn_enable(pin_state_t newShutDownState){
    HAL_GPIO_WritePin(LTC4421_SHDN_PORT, LTC4421_SHDN_PIN, newShutDownState);
}