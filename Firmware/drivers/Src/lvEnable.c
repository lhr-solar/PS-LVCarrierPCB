#include "lvEnable.h"

void lvEnable_gpio_init(void){
    gpio_pin_init(LV_EN_SUPP_PORT, LV_EN_SUPP_PIN, INPUT);
    gpio_pin_init(LV_EN_POWERSUPPLY_PORT, LV_EN_POWERSUPPLY_PIN, INPUT);
}

pin_state_t get_lvEnable_supp(void){
    // lv enable supp is negative logic
    return HAL_GPIO_ReadPin(LV_EN_SUPP_PORT, LV_EN_SUPP_PIN) == GPIO_PIN_SET ? OFF : ON;
}

pin_state_t get_lvEnable_powerSupply(void){
    // lv enable power supply is negative logic
    return HAL_GPIO_ReadPin(LV_EN_POWERSUPPLY_PORT, LV_EN_POWERSUPPLY_PIN) == GPIO_PIN_SET ? OFF : ON;
}