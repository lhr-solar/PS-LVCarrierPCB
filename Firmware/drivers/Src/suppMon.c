#include "suppMon.h"
#include "ADC.h" // Embedded-Sharepoint ADC driver

#define ADC1_QUEUE_LENGTH   10
#define ADC_ITEM_SIZE       sizeof( uint32_t )

static uint8_t adc1_static_storage[ADC1_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t adc1QueueBuffer;
static QueueHandle_t adc1RecvQ;


adc_status_t lv_carrier_adc_init(void){
    adc1RecvQ = xQueueCreateStatic( ADC1_QUEUE_LENGTH, ADC_ITEM_SIZE, adc1_static_storage, &adc1QueueBuffer);

    if (adc1RecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    return ADC_OK;

}


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(adcHandle->Instance==ADC1)
  {
  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB0     ------> ADC1_IN15
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
  if(adcHandle->Instance==ADC1)
  {
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB0     ------> ADC1_IN15
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

    HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  }
}

