#include "ltc4421.h"

void ltc4421_gpio_init(void){
    
    gpio_pin_init(LTC4421_SHDN_PORT, LTC4421_SHDN_PIN, OUTPUT);
    
    // Channel selected pins
    gpio_pin_init(LTC4421_CHANNEL1_SELECTED_PORT, LTC4421_CHANNEL1_SELECTED_PIN, INPUT);
    gpio_pin_init(LTC4421_CHANNEL2_SELECTED_PORT, LTC4421_CHANNEL2_SELECTED_PIN, INPUT);

    // Valid channel pins
    gpio_pin_init(LTC4421_VALID1_SELECTED_PORT, LTC4421_VALID1_SELECTED_PIN, INPUT);
    gpio_pin_init(LTC4421_VALID2_SELECTED_PORT, LTC4421_VALID2_SELECTED_PIN, INPUT);

    
    // Fault channel pins
    gpio_pin_init(LTC4421_FAULT1_PORT, LTC4421_FAULT1_PIN, INPUT);
    gpio_pin_init(LTC4421_FAULT2_PORT, LTC4421_FAULT2_PIN, INPUT);

}

void ltc4421_shdn_enable(pin_state_t newShutDownState){
    HAL_GPIO_WritePin(LTC4421_SHDN_PORT, LTC4421_SHDN_PIN, newShutDownState);
}

pin_state_t ltc4421_read_valid(ltc4421_channel_t channel){

    pin_state_t ret = OFF;
    switch(channel){
        case CHANNEL1:
            ret = HAL_GPIO_ReadPin(LTC4421_VALID1_SELECTED_PORT, LTC4421_VALID1_SELECTED_PIN) == GPIO_PIN_SET ? ON : OFF;
            break;
        case CHANNEL2:
            ret = HAL_GPIO_ReadPin(LTC4421_VALID2_SELECTED_PORT, LTC4421_VALID2_SELECTED_PIN) == GPIO_PIN_SET ? ON : OFF;
            break;
        default:
            break;
    }
    return ret;
}


ltc4421_channel_t ltc4421_channel_selected(void){

    if(HAL_GPIO_ReadPin(LTC4421_CHANNEL1_SELECTED_PORT, LTC4421_CHANNEL1_SELECTED_PIN) == GPIO_PIN_SET){
        return CHANNEL1;
    }
    else if(HAL_GPIO_ReadPin(LTC4421_CHANNEL2_SELECTED_PORT, LTC4421_CHANNEL2_SELECTED_PIN) == GPIO_PIN_SET){
        return CHANNEL2;
    }
    return NO_CHANNEL;
}

pin_state_t ltc4421_read_fault(ltc4421_channel_t channel){

    pin_state_t ret = OFF;
    switch(channel){
        case CHANNEL1:
            ret = HAL_GPIO_ReadPin(LTC4421_FAULT1_PORT, LTC4421_FAULT1_PIN) == GPIO_PIN_SET ? ON : OFF;
            break;
        case CHANNEL2:
            ret = HAL_GPIO_ReadPin(LTC4421_FAULT2_PORT, LTC4421_FAULT2_PIN) == GPIO_PIN_SET ? ON : OFF;
            break;
        default:
            break;
    }
    return ret;
}