#include "addressableLED.h"


TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch1;

static uint8_t ledData[MAX_RGB_LEDS][NUMBER_PWM_DATA_ELEMENTS];
static uint16_t pwmData[(24 * MAX_RGB_LEDS) + WS2812_RESET_TIME];

// functions generated from CubeMX
void adressable_led_tim4_init(void);
void addressable_led_dma_init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle);

void addressableLED_init(void){

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    adressable_led_tim4_init();
    addressable_led_dma_init();

    ws2812b_init(
        &wsHandle,
        ledData,
        pwmData,
        &htim4,
        TIM_CHANNEL_1,
        MAX_RGB_LEDS
    );

    ws2812b_set_all_leds(&wsHandle, WS2812B_SOLID_OFF, portMAX_DELAY);
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (htim->Instance == htim4.Instance)
    {
        ws2812b_TIM_PWM_PulseFinishedCallback(htim, &wsHandle, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// You will need this for the DMA interrupt to process properly
void DMA1_Channel1_IRQHandler(){
    HAL_DMA_IRQHandler(&hdma_tim4_ch1);
}

void addressable_led_dma_init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+4, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==htim4.Instance)
  {
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 DMA Init */
    /* TIM4_CH1 Init */
    hdma_tim4_ch1.Instance = DMA1_Channel1;
    hdma_tim4_ch1.Init.Request = DMA_REQUEST_TIM4_CH1;
    hdma_tim4_ch1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim4_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim4_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim4_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.Mode = DMA_NORMAL;
    hdma_tim4_ch1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_tim4_ch1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(tim_pwmHandle,hdma[TIM_DMA_ID_CC1],hdma_tim4_ch1);
  }
}

/* TIM4 init function */
void adressable_led_tim4_init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim4);

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM4)
  {

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  }

}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM4)
  {
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /* TIM4 DMA DeInit */
    HAL_DMA_DeInit(tim_pwmHandle->hdma[TIM_DMA_ID_CC1]);
  }
}