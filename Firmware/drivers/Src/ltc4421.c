#include "ltc4421.h"


void ltc4421_gpio_init(void){
    gpio_pin_init(LTC4421_SHDN_PORT, LTC4421_SHDN_PIN, OUTPUT);
}

void ltc4421_shdn_enable(pin_state_t newShutDownState){
    HAL_GPIO_WritePin(LTC4421_SHDN_PORT, LTC4421_SHDN_PIN, newShutDownState);
}