#include "stm32xx_hal.h"
#include "bq25756e.h"

void blinky_gpio_init(void)
{
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = LSOM_HEARTBEAT_PIN};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_Init(LSOM_HEARTBEAT_PORT, &led_config);
}

int main()
{
    HAL_Init();
    SystemClock_Config();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    blinky_gpio_init();
    bq25756e_init();
    while (1)
    {
        bq25756e_transmit(START);

        HAL_GPIO_TogglePin(LSOM_HEARTBEAT_PORT, LSOM_HEARTBEAT_PIN);
        HAL_Delay(500);
    }
}
