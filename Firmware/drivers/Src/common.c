#include "pinDefs.h"
#include "common.h"

// void blinky_gpio_init(void)
// {
//     // on lsom
//     GPIO_InitTypeDef led_config = {
//         .Mode = GPIO_MODE_OUTPUT_PP,
//         .Pull = GPIO_NOPULL,
//         .Pin = LSOM_HEARTBEAT_PIN};

//     __HAL_RCC_GPIOC_CLK_ENABLE();
//     HAL_GPIO_Init(LSOM_HEARTBEAT_PORT, &led_config);
// }

void gpio_clock_init(GPIO_TypeDef *port){

    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
}

void gpio_pin_init(GPIO_TypeDef *port, uint32_t pin, pin_mode_t pinMode){

    // init the port's clock
    gpio_clock_init(port);

    GPIO_InitTypeDef pin_config = {
        .Mode = pinMode,
        .Pull = GPIO_NOPULL,
        .Pin = pin
    };

    HAL_GPIO_Init(port, &pin_config);
}
