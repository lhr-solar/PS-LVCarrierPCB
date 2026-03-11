#include "adc_sense.h"

#define ADC_ISR_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#define ADC_ITEM_SIZE       sizeof( uint32_t )

#define SUPP_QUEUE_LENGTH  ADC_QUEUE_LENGTH
#define SUPP_REG_QUEUE_LENGTH   ADC_QUEUE_LENGTH

#define COMBINED_LENGTH ( SUPP_QUEUE_LENGTH + SUPP_REG_QUEUE_LENGTH )

static uint8_t suppVoltage_static_storage[SUPP_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppVoltageQueueBuffer;
static QueueHandle_t suppVoltageRecvQ;

static uint8_t suppReg_static_storage[SUPP_REG_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppRegQueueBuffer;
static QueueHandle_t suppRegRecvQ;


static uint8_t suppCurrent_static_storage[SUPP_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppCurrentQueueBuffer;
static QueueHandle_t suppCurrentRecvQ;

static uint8_t suppRegCurrent_static_storage[SUPP_REG_QUEUE_LENGTH * ADC_ITEM_SIZE];
static StaticQueue_t suppRegCurrentQueueBuffer;
static QueueHandle_t suppRegCurrentRecvQ;


// Queue Set Definitions
static QueueSetHandle_t suppQSet;
static StaticQueue_t static_suppQset;
static uint8_t suppQSet_static_storage[ COMBINED_LENGTH * ADC_ITEM_SIZE ];

static QueueSetHandle_t suppRegQSet;
static StaticQueue_t static_suppRegQSet;
static uint8_t suppRegQSet_static_storage[ COMBINED_LENGTH * ADC_ITEM_SIZE ];

// Semaphores for Queue Set Arbitration
SemaphoreHandle_t supp_adc_semphr = NULL;
StaticSemaphore_t supp_semphr_buffer;

SemaphoreHandle_t reg_adc_semphr = NULL;
StaticSemaphore_t reg_semphr_buffer;

// ADC1
static ADC_ChannelConfTypeDef suppCurrent_adc_cfg = {
    .Channel = ADC_CHANNEL_15,          // PB0
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

// ADC1
static ADC_ChannelConfTypeDef suppReg_adc_cfg = {
    .Channel = ADC_CHANNEL_3,           // PA2
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

// ADC3
static ADC_ChannelConfTypeDef suppRegCurrent_adc_cfg = {
    .Channel = ADC_CHANNEL_1,          // PB1
    .Rank = ADC_REGULAR_RANK_1,
    .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
    .SingleDiff = ADC_SINGLE_ENDED,
    .OffsetNumber = ADC_OFFSET_NONE,
    .Offset = 0
};

// ADC2
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

adc_status_t adc3_init(void){

  ADC_MultiModeTypeDef multimode = {0};

  hadc3->Instance = ADC3;
  hadc3->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc3->Init.Resolution = ADC_RESOLUTION_12B;
  hadc3->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3->Init.GainCompensation = 0;
  hadc3->Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3->Init.LowPowerAutoWait = DISABLE;
  hadc3->Init.ContinuousConvMode = DISABLE;
  hadc3->Init.NbrOfConversion = 1;
  hadc3->Init.DiscontinuousConvMode = DISABLE;
  hadc3->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3->Init.DMAContinuousRequests = DISABLE;
  hadc3->Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3->Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(hadc3) != HAL_OK)
  {
    return ADC_INIT_FAIL;
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(hadc3, &multimode) != HAL_OK)
  {
    return ADC_INIT_FAIL;
  }

  HAL_ADCEx_Calibration_Start(hadc1, ADC_SINGLE_ENDED);


  return ADC_OK;
}

adc_status_t adc_sense_init(void){
  
    suppVoltageRecvQ = xQueueCreateStatic( ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppVoltage_static_storage, &suppVoltageQueueBuffer);
    if (suppVoltageRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppRegRecvQ = xQueueCreateStatic( ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppReg_static_storage, &suppRegQueueBuffer);
    if (suppRegRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppCurrentRecvQ = xQueueCreateStatic( ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppCurrent_static_storage, &suppCurrentQueueBuffer);
    if (suppCurrentRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    suppRegCurrentRecvQ = xQueueCreateStatic( ADC_QUEUE_LENGTH, ADC_ITEM_SIZE, suppRegCurrent_static_storage, &suppRegCurrentQueueBuffer);
    if (suppRegCurrentRecvQ == NULL) {
        return ADC_INIT_FAIL;
    }

    // Init Queue Sets
    suppQSet = xQueueCreateSetStatic( COMBINED_LENGTH, suppQSet_static_storage, &static_suppQset );
    if (suppQSet == NULL) {
        return ADC_INIT_FAIL;
    }
    suppRegQSet = xQueueCreateSetStatic( COMBINED_LENGTH, suppRegQSet_static_storage, &static_suppRegQSet );
    if (suppRegQSet == NULL) {
        return ADC_INIT_FAIL;
    }

    xQueueAddToSet(suppVoltageRecvQ, suppQSet);
    xQueueAddToSet(suppCurrentRecvQ, suppQSet);

    xQueueAddToSet(suppRegRecvQ, suppRegQSet);
    xQueueAddToSet(suppRegCurrentRecvQ, suppRegQSet);

    supp_adc_semphr = xSemaphoreCreateBinaryStatic( &supp_semphr_buffer );
    reg_adc_semphr = xSemaphoreCreateBinaryStatic( &reg_semphr_buffer );
    xSemaphoreGive(supp_adc_semphr);
    xSemaphoreGive(reg_adc_semphr);

    if (adc1_init() != ADC_OK){
      return ADC_INIT_FAIL;
    }

    if (adc2_init() != ADC_OK){
      return ADC_INIT_FAIL;
    }

    if(adc3_init() != ADC_OK){
      return ADC_INIT_FAIL;
    }

    return ADC_OK;

}

adc_status_t adc_start_read(adc_sense_channel_t channel, TickType_t delay_ticks)
{
  switch (channel)
  {
    case SUPPLEMENTAL_BATTERY_VOLTAGE:
        xSemaphoreTake(supp_adc_semphr, delay_ticks);
        return adc_read(hadc2, &suppVoltage_adc_cfg, suppVoltageRecvQ);
        

    case SUPPLEMENTAL_BATTERY_CURRENT:
        xSemaphoreTake(supp_adc_semphr, delay_ticks);
        return adc_read(hadc1, &suppCurrent_adc_cfg, suppCurrentRecvQ);

    case REGULATED_BATTERY_VOLTAGE:
        xSemaphoreTake(reg_adc_semphr, delay_ticks);
        return adc_read(hadc1, &suppReg_adc_cfg, suppRegRecvQ);

    case REGULATED_BATTERY_CURRENT:
        xSemaphoreTake(reg_adc_semphr, delay_ticks);
        return adc_read(hadc2, &suppRegCurrent_adc_cfg, suppRegCurrentRecvQ);

    default:
        return ADC_CHANNEL_CONFIG_FAIL;
  }
}

BaseType_t adc_read_value(adc_sense_channel_t channel, uint32_t *result, TickType_t delay_ticks){
  BaseType_t readStatus = pdFALSE;
  QueueSetMemberHandle_t activeQ;

  if (channel == SUPPLEMENTAL_BATTERY_VOLTAGE || channel == SUPPLEMENTAL_BATTERY_CURRENT) {
    activeQ = xQueueSelectFromSet( suppQSet, delay_ticks );
    if (activeQ == suppVoltageRecvQ) {
      
      if (channel == SUPPLEMENTAL_BATTERY_VOLTAGE) {
        // signal done conversion
        readStatus = xQueueReceive(suppVoltageRecvQ, result, delay_ticks);
        if (readStatus == pdPASS)  xSemaphoreGive(supp_adc_semphr); 

      } else {
        // wait for conversion
        readStatus = xQueueReceive(suppCurrentRecvQ, result, delay_ticks);
        if (readStatus == pdPASS)  xSemaphoreGive(supp_adc_semphr);
        
      }
    } else if (activeQ == suppCurrentRecvQ) {
     
      if (channel == SUPPLEMENTAL_BATTERY_CURRENT) {
        // signal done conversion
        readStatus = xQueueReceive(suppCurrentRecvQ, result, delay_ticks);
        if (readStatus == pdPASS) xSemaphoreGive(supp_adc_semphr);
        
      } else {
        // wait for conversion
        readStatus = xQueueReceive(suppVoltageRecvQ, result, delay_ticks);
        if (readStatus == pdPASS)  xSemaphoreGive(supp_adc_semphr); 

      }
    }

  } else if (channel == REGULATED_BATTERY_VOLTAGE || channel == REGULATED_BATTERY_CURRENT) {
    activeQ = xQueueSelectFromSet( suppRegQSet, delay_ticks );
    if (activeQ == suppRegRecvQ) {

      if (channel == REGULATED_BATTERY_VOLTAGE) {
        // signal done conversion
        readStatus = xQueueReceive(suppRegRecvQ, result, delay_ticks);
        if (readStatus == pdPASS) xSemaphoreGive(reg_adc_semphr);
        
      } else {
        // wait for conversion
        readStatus = xQueueReceive(suppRegCurrentRecvQ, result, delay_ticks);
        if (readStatus == pdPASS) xSemaphoreGive(reg_adc_semphr);
      
      }
    } else if (activeQ == suppRegCurrentRecvQ) {
      
      if (channel == REGULATED_BATTERY_CURRENT) {
        // signal done conversion
        readStatus = xQueueReceive(suppRegCurrentRecvQ, result, delay_ticks);
        if (readStatus == pdPASS) xSemaphoreGive(reg_adc_semphr);
        
      } else {
        // wait for conversion
        readStatus = xQueueReceive(suppRegRecvQ, result, delay_ticks);
        if (readStatus == pdPASS) xSemaphoreGive(reg_adc_semphr);

      }
    }
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
    /**ADC2 GPIO Configuration
    PA1     ------> ADC2_IN2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC2 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, ADC_ISR_PRIO, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC2_MspInit 1 */

  /* USER CODE END ADC2_MspInit 1 */
  }
  else if(adcHandle->Instance==ADC3)
  {

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC345;
    PeriphClkInit.Adc345ClockSelection = RCC_ADC345CLKSOURCE_SYSCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC3 clock enable */
    __HAL_RCC_ADC345_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC3 GPIO Configuration
    PB1     ------> ADC3_IN1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    /* ADC2 interrupt Deinit */
  }
  else if(adcHandle->Instance==ADC3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ADC345_CLK_DISABLE();

    /**ADC3 GPIO Configuration
    PB1     ------> ADC3_IN1
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);
  }
}
