#include "stm32xx_hal.h"
#include "bq25756e.h"

int main()
{
    HAL_Init();
    SystemClock_Config();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    blinky_gpio_init();
    bq25756e_init();

    // pull Pb4 low --- charge enable pin
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /*Configure GPIO pin : PB4 */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);


    while (1)
    {
        bq25756e_transmit(START);

        HAL_GPIO_TogglePin(LSOM_HEARTBEAT_PORT, LSOM_HEARTBEAT_PIN);
        HAL_Delay(500);
    }
}
