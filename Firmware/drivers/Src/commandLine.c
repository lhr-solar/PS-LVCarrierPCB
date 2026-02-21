#include "pinDefs.h"
#include "commandLine.h"


#define LSOM_U

void command_line_init(void)
{
    HAL_GPIO_TogglePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN);

    husart3->Init.BaudRate = 115200;
    husart3->Init.WordLength = UART_WORDLENGTH_8B;
    husart3->Init.StopBits = UART_STOPBITS_1;
    husart3->Init.Parity = UART_PARITY_NONE;
    husart3->Init.Mode = UART_MODE_TX_RX;
    husart3->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    husart3->Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_GPIO_TogglePin(LSOM_HEARTBEAT_LED_PORT, LSOM_HEARTBEAT_LED_PIN);

    printf_init(husart3);
}

#ifdef LSOM_S
void HAL_UART_MspGPIOInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* enable port B USART3 gpio
    PB10 -> USART3_TX
    PB11 -> USART3_RX
    */
    init.Pin = USB_UART_TX | USB_UART_RX;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    init.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(USB_UART_CLOCK_PORT, &init);
}

#else
void HAL_UART_MspGPIOInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef init = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PC10     ------> USART3_TX
    PC11     ------> USART3_RX
    */
    init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    init.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOC, &init);
}
#endif
