#include "statusLeds.h"


void statusLeds_init(void){

    gpio_pin_init(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, OUTPUT); // initialize heartbeat led on OBBB
    gpio_pin_init(SUPPBAT_FAULT_PORT, SUPPBAT_FAULT_PIN, OUTPUT); // initialize suppBat fault led on OBBB
    gpio_pin_init(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN, OUTPUT); // initialize heartbeat led on LSOM

}

void statusLeds_write(status_leds_t led, pin_state_t newState){
    switch(led){
        case LSOM_HEARTBEAT_LED:
            HAL_GPIO_WritePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN, newState);
            break;
        case HEARTBEAT_LED:
            HAL_GPIO_WritePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, newState);
            break;
        case SUPPBATT_FAULT_LED:
            HAL_GPIO_WritePin(SUPPBAT_FAULT_PORT, SUPPBAT_FAULT_PIN, newState);
            break;
        default:
            break;
    }
}

void statusLeds_toggle(status_leds_t led){
    switch(led){
        case LSOM_HEARTBEAT_LED:
            HAL_GPIO_TogglePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN);
            break;
        case HEARTBEAT_LED:
            HAL_GPIO_TogglePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
            break;
        case SUPPBATT_FAULT_LED:
            HAL_GPIO_TogglePin(SUPPBAT_FAULT_PORT, SUPPBAT_FAULT_PIN);
            break;
        default:
            break;
    }
}
