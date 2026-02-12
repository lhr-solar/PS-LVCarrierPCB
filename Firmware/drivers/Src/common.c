#include "common.h"
#include "pinDefs.h"

void blinky_gpio_init(void)
{
    // on lsom
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = LSOM_HEARTBEAT_PIN};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_Init(LSOM_HEARTBEAT_PORT, &led_config);
}

void gpio_clock_init(uint32_t port){
    switch (port) {
        case (uint32_t)GPIOA:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        case (uint32_t)GPIOB:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        case (uint32_t)GPIOC:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        default:
            break;
    }
}