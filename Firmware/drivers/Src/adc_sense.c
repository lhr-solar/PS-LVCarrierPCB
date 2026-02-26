#include "adc_sense.h"

#define ADC_ISR_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#define ADC_ITEM_SIZE       sizeof( uint32_t )

static uint8_t suppVoltage_static_storage[SUPP_VOLTAGE_ADC_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppVoltageQueueBuffer;
static QueueHandle_t suppVoltageRecvQ;

static uint8_t suppReg_static_storage[SUPP_REGULATED_ADC_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppRegQueueBuffer;
static QueueHandle_t suppRegRecvQ;


static uint8_t suppCurrent_static_storage[SUPP_HALL_ADC_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppCurrentQueueBuffer;
static QueueHandle_t suppCurrentRecvQ;

static uint8_t suppRegCurrent_static_storage[SUPP_REGULATED_HALL_ADC_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppRegCurrentQueueBuffer;
static QueueHandle_t suppRegCurrentRecvQ;

static ADC_ChannelConfTypeDef suppCurrent_adc_cfg = {
    .Channel = ADC_CHANNEL_15,          // PB0
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

static ADC_ChannelConfTypeDef suppReg_adc_cfg = {
    .Channel = ADC_CHANNEL_3,           // PA2
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

static ADC_ChannelConfTypeDef suppRegCurrent_adc_cfg = {
    .Channel = ADC_CHANNEL_12,          // PB2
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

static ADC_ChannelConfTypeDef suppVoltage_adc_cfg = {
    .Channel = ADC_CHANNEL_2,           // PA1
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};


static adc_status_t adc1_init(){
  
  ADC_MultiModeTypeDef multimode = {0};
  
  /** Common config
  */
  hadc1->Instance = ADC1;
  hadc1->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1->Init.Resolution = ADC_RESOLUTION_12B;
  hadc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1->Init.GainCompensation = 0;
  hadc1->Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1->Init.LowPowerAutoWait = DISABLE;
  hadc1->Init.ContinuousConvMode = DISABLE;
  hadc1->Init.NbrOfConversion = 1;
  hadc1->Init.DiscontinuousConvMode = DISABLE;
  hadc1->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1->Init.DMAContinuousRequests = DISABLE;
  hadc1->Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1->Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(hadc1) != HAL_OK)
  {
    return ADC_INIT_FAIL;
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(hadc1, &multimode) != HAL_OK)
  {
    return ADC_INIT_FAIL;
  }

  HAL_ADCEx_Calibration_Start(hadc1, ADC_SINGLE_ENDED);


  return ADC_OK;

}

static adc_status_t adc2_init(){


  hadc2->Instance = ADC2;
  hadc2->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2->Init.Resolution = ADC_RESOLUTION_12B;
  hadc2->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2->Init.GainCompensation = 0;
  hadc2->Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2->Init.LowPowerAutoWait = DISABLE;
  hadc2->Init.ContinuousConvMode = DISABLE;
  hadc2->Init.NbrOfConversion = 1;
  hadc2->Init.DiscontinuousConvMode = DISABLE;
  hadc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2->Init.DMAContinuousRequests = DISABLE;
  hadc2->Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2->Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(hadc2) != HAL_OK)
  {
     return ADC_INIT_FAIL;
  }

  HAL_ADCEx_Calibration_Start(hadc2, ADC_SINGLE_ENDED);


  return ADC_OK;
}

adc_status_t adc_sense_init(void){


    suppVoltageRecvQ = xQueueCreateStatic( SUPP_VOLTAGE_ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppVoltage_static_storage, &suppVoltageQueueBuffer);
    if (suppVoltageRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppRegRecvQ = xQueueCreateStatic( SUPP_REGULATED_ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppReg_static_storage, &suppRegQueueBuffer);
    if (suppRegRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppCurrentRecvQ = xQueueCreateStatic( SUPP_HALL_ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppCurrent_static_storage, &suppCurrentQueueBuffer);
    if (suppCurrentRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppRegCurrentRecvQ = xQueueCreateStatic( SUPP_REGULATED_HALL_ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppRegCurrent_static_storage, &suppRegCurrentQueueBuffer);
    if (suppRegCurrentRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    if (adc1_init() != ADC_OK)
        return ADC_INIT_FAIL;

    if (adc2_init() != ADC_OK)
        return ADC_INIT_FAIL;

    return ADC_OK;

}

adc_status_t adc_start_read(adc_sense_channel_t channel)
{
  switch (channel)
  {
    case SUPPLEMENTAL_BATTERY_VOLTAGE:
        return adc_read(hadc2, &suppVoltage_adc_cfg, suppVoltageRecvQ);

    case SUPPLEMENTAL_BATTERY_CURRENT:
        return adc_read(hadc1, &suppCurrent_adc_cfg, suppCurrentRecvQ);

    case REGULATED_BATTERY_VOLTAGE:
        return adc_read(hadc1, &suppReg_adc_cfg, suppRegRecvQ);

    case REGULATED_BATTERY_CURRENT:
        return adc_read(hadc2, &suppRegCurrent_adc_cfg, suppRegCurrentRecvQ);

    default:
        return ADC_CHANNEL_CONFIG_FAIL;
  }
}

BaseType_t adc_read_value(adc_sense_channel_t channel, uint32_t *result, TickType_t delay_ticks){
  BaseType_t readStatus = pdFALSE;
  switch (channel)
  {
    case SUPPLEMENTAL_BATTERY_VOLTAGE:
        readStatus = xQueueReceive(suppVoltageRecvQ, result, delay_ticks);
        break;

    case SUPPLEMENTAL_BATTERY_CURRENT:
        readStatus = xQueueReceive(suppCurrentRecvQ, result, delay_ticks);
        break;

    case REGULATED_BATTERY_VOLTAGE:

        readStatus = xQueueReceive(suppRegRecvQ, result, delay_ticks);

        break;

    case REGULATED_BATTERY_CURRENT:
        readStatus = xQueueReceive(suppRegCurrentRecvQ, result, delay_ticks);
        break;

    default:
        readStatus = pdFALSE;
  }
  return readStatus;

}


static uint32_t HAL_RCC_ADC12_CLK_ENABLED=0;

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
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA2     ------> ADC1_IN3
    PB0     ------> ADC1_IN15
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, ADC_ISR_PRIO, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
  else if(adcHandle->Instance==ADC2)
  {

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC2 clock enable */
    HAL_RCC_ADC12_CLK_ENABLED++;
    if(HAL_RCC_ADC12_CLK_ENABLED==1){
      __HAL_RCC_ADC12_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC2 GPIO Configuration
    PA1     ------> ADC2_IN2
    PB2     ------> ADC2_IN12
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* ADC2 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, ADC_ISR_PRIO, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {

    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC1 GPIO Configuration
    PA2     ------> ADC1_IN3
    PB0     ------> ADC1_IN15
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

  }
  else if(adcHandle->Instance==ADC2)
  {
    /* Peripheral clock disable */
    HAL_RCC_ADC12_CLK_ENABLED--;
    if(HAL_RCC_ADC12_CLK_ENABLED==0){
      __HAL_RCC_ADC12_CLK_DISABLE();
    }

    /**ADC2 GPIO Configuration
    PA1     ------> ADC2_IN2
    PB2     ------> ADC2_IN12
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);
  }
}
