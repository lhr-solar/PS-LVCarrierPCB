#include "bq25756e.h"

// Size of bus payloads
#define BQ_RX_SIZE 1
#define BQ_TX_SIZE 2

// BQ Handle
static BQ_HandleTypeDef* bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef* bq_i2c_handle=&hi2c4;

// I2C Bus Semaphore
SemaphoreHandle_t bq_i2c_smphr = NULL;
static StaticSemaphore_t xSemaphoreBuffer;

/* Write to a register on chip (transmit w/ 2 bytes) */
static bq25756e_status_t bq25756e_write_reg(uint8_t reg, uint8_t data, TickType_t delay);
/* Read a register on chip (transmit + receive) */
static bq25756e_status_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer, TickType_t delay);

static bq25756e_status_t bq25756e_RevMode_disable(TickType_t delay);
static bq25756e_status_t bq25756e_TempSense_disable(TickType_t delay);
static bq25756e_status_t bq25756e_HiZ_disable(TickType_t delay);
static bq25756e_status_t bq25756e_SW_Ichg_enable(TickType_t delay, bq25756e_chg_current_t limit);
static bq25756e_status_t bq25756e_HW_Ichg_disable(TickType_t delay);

static void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring);
static void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring);

static void bq25756e_gpio_init(void);

bq25756e_status_t bq25756e_charge(TickType_t delay, bq25756e_chg_current_t limit) {
    bq25756e_status_t stat=BQ25756E_OK;

    // Disable Charge Limit
    stat=bq25756e_HW_Ichg_disable(delay);
    if (stat != BQ25756E_OK) return stat;
    // Write Charge Limit
    stat=bq25756e_SW_Ichg_enable(delay, limit);
    if (stat != BQ25756E_OK) return stat;
    // Disable Hi Z
    stat=bq25756e_HiZ_disable(delay);
    if (stat != BQ25756E_OK) return stat;
    // Disable Temp Sense
    stat=bq25756e_TempSense_disable(delay);
    if (stat != BQ25756E_OK) return stat;
    // Disable Rev Mode 
    stat=bq25756e_RevMode_disable(delay);
    if (stat != BQ25756E_OK) return stat;
    // Pull CE Pin low to start charging
    bq25756e_write_ce(BQ25756E_LOGIC_LOW);

    return stat;
}

bq25756e_status_t bq25756e_pet_wdg(TickType_t delay) {
  uint8_t buff[1];

  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_assert_bits(buff, BQ25756E_BIT_WDG_RESET);
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_dump_status(TickType_t delay){
  uint8_t buff[1];

  // Dump charger status
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_STATUS_1, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  
  printf("Charge Status: ");
  uint8_t charge_stat = buff[0] & BQ25756E_BIT_CHARGE_STAT;
  switch (charge_stat) {
    case 0:
      printf("Not charging. \n\r"); break;
    case 1:
      printf("Trickle Charge. \n\r"); break;
    case 2:
      printf("Pre-Charge. \n\r"); break;
    case 3:
      printf("Fast Charge. \n\r"); break;
    case 4:
      printf("Taper Charge. \n\r"); break;
    case 5:
      printf("y is it here lmao ur cooked. \n\r"); break;
    case 6:
      printf("Top off Timer Charge. \n\r"); break;
    case 7:
      printf("Charge Termination Done. \n\r"); break;
    default: break;
  }

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_dump_faults(TickType_t delay) {
  uint8_t buff[1];
  uint8_t fault_cnt=0;

  if (bq25756e_read_reg(BQ25756E_REG_FAULT_STATUS, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }

  printf("Faults: \n-------\n");
  if (buff[0] & BQ25756E_BIT_INPUT_UV_FAULT) {
    printf("Input Undervoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_INPUT_OV_FAULT) {
    printf("Input Overvoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_BATT_OVERCURRENT_FAULT) {
    printf("Battery Overcurrent! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_BATT_OV_FAULT) {
    printf("Battery Overvoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_THERMAL_SHDN_FAULT) {
    printf("Thermal Shutdown! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_DRV_SUP_FAULT) {
    printf("Thermal Shutdown! \n");
    fault_cnt++;
  }
  printf("Num Faults: %d\n-------\n\r", fault_cnt);

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_charge_disable(TickType_t delay) {
  uint8_t buff[1];

  // Charge disable
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_CHARGE_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  // Disable CE
  bq25756e_write_ce(BQ25756E_LOGIC_HIGH);

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_init(void) {   
  bq25756e_status_t stat;
  bq25756e_gpio_init();

  // init semphr
  bq_handle->bq_i2c_smphr = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );
  bq_handle->hi2c = bq_i2c_handle;
  bq_handle->device_id = DEVICE_ADDR;

  stat=bq25756e_i2c4_init();

  // init pre req bits
  bq25756e_preReqBits_init();

  return stat;
}

void bq25756e_write_ce(bq25756e_logic_t value) {
  // SW write to charge enable pin 
  HAL_GPIO_WritePin(BQ25756E_CE_PORT, BQ25756E_CE_PIN, value);
}

/****************************** STATIC DRIVER HELPERS ********************************/

static bq25756e_status_t bq25756e_RevMode_disable(TickType_t delay) {
  // Disable reverse mode (battery -> input)
  uint8_t buff[1];
  if (bq25756e_read_reg(BQ25756E_REG_REVERSE_MODE, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_REVERSE_MODE_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_REVERSE_MODE, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  } 

  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_TempSense_disable(TickType_t delay) {
  // Disables temperature sense (HW temp sense is pulled low)
  uint8_t buff[1];
  if (bq25756e_read_reg(BQ25756E_REG_TEMP, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_TEMP_SENSE_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_TEMP, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_HiZ_disable(TickType_t delay) {
  // Disables HiZ mode
  uint8_t buff[1];
  if (bq25756e_read_reg(BQ25756E_REG_PIN_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_HIZ_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_PIN_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_SW_Ichg_enable(TickType_t delay, bq25756e_chg_current_t limit) {
  // Enables SW charge limit based on macros
  uint8_t buff1[1], buff2[1];
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_B, buff1, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x06 
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_A, buff2, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x40 

  bq25756e_clear_bits(buff1, BQ25756E_BIT_CHARGE_CURRENT_FIELD_A); 
  bq25756e_clear_bits(buff2, BQ25756E_BIT_CHARGE_CURRENT_FIELD_B);

  uint8_t mask, mask_b, mask_a; // split among 2 registers 0x03 and 0x02

  mask=limit;
  
  mask_b=(uint8_t) (mask >> 0x08);
  mask_a=(uint8_t) (mask & 0xFF);
  bq25756e_assert_bits(buff1, mask_a);
  bq25756e_assert_bits(buff2, mask_b);

  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_B, buff1[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }  
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_A, buff2[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_HW_Ichg_disable(TickType_t delay) {
  // Disables hardware charge limit
  uint8_t buff[1];
  if (bq25756e_read_reg(BQ25756E_REG_PIN_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_CHARGE_LIMIT_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_PIN_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer, TickType_t delay) {
  // TX
  // shift DEVICE_ADDR to have space for R/W bit
  if ( HAL_I2C_Master_Transmit_IT(&hi2c4, (DEVICE_ADDR << 1), &reg, BQ_RX_SIZE) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }
  if (xSemaphoreTake(bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  // RX
  if ( HAL_I2C_Master_Receive_IT(&hi2c4, (DEVICE_ADDR << 1), buffer, BQ_RX_SIZE) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }
  if (xSemaphoreTake(bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_write_reg(uint8_t reg, uint8_t data, TickType_t delay) {
  // TX
  uint8_t payload[BQ_TX_SIZE];
  payload[0]=reg;
  payload[1]=data;

  if (HAL_I2C_Master_Transmit_IT (&hi2c4, (DEVICE_ADDR << 1), payload, BQ_TX_SIZE) != HAL_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  if (xSemaphoreTake(bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  return BQ25756E_OK;
}

void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  &  ~(bitstring));
}

void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  |  (bitstring));
}

static void bq25756e_gpio_init(void)
{
  // Init Charge Enable pin
  gpio_pin_init(BQ25756E_CE_PORT, BQ25756E_CE_PIN, OUTPUT);
  // Disable CE pin on startup
  bq25756e_write_ce(BQ25756E_LOGIC_HIGH);  
  gpio_pin_init(BQ25756E_STAT2_PORT, BQ25756E_STAT2_PIN, INPUT);
  gpio_pin_init(BQ25756E_STAT1_PORT, BQ25756E_STAT1_PIN, INPUT);
  gpio_pin_init(BQ25756E_INT_PORT, BQ25756E_INT_PIN, INPUT);
}

/****************************** I2C CALLBACKS ********************************/

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
     statusLeds_toggle(LSOM_HEARTBEAT_LED);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xSemaphoreGiveFromISR(bq_i2c_smphr, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
     statusLeds_toggle(LSOM_HEARTBEAT_LED);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xSemaphoreGiveFromISR(bq_i2c_smphr, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    // kinda scuffed cuz if NAK hits this callback so still release
     statusLeds_toggle(LSOM_HEARTBEAT_LED);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    xSemaphoreGiveFromISR(bq_i2c_smphr, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

bq25756e_status_t bq25756e_i2c4_init(void)
{

  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00503D58;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  /** Configure Analog filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  return BQ25756E_OK;
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hi2c->Instance==I2C4)
  {
    /* USER CODE BEGIN I2C4_MspInit 0 */

    /* USER CODE END I2C4_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
    PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2C4 GPIO Configuration
    PC6     ------> I2C4_SCL
    PC7     ------> I2C4_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_I2C4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_I2C4_CLK_ENABLE();
    /* I2C4 interrupt Init */
    HAL_NVIC_SetPriority(I2C4_EV_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_SetPriority(I2C4_ER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
    /* USER CODE BEGIN I2C4_MspInit 1 */

    /* USER CODE END I2C4_MspInit 1 */

  }

}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C4)
  {
    /* USER CODE BEGIN I2C4_MspDeInit 0 */

    /* USER CODE END I2C4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C4_CLK_DISABLE();

    /**I2C4 GPIO Configuration
    PC6     ------> I2C4_SCL
    PC7     ------> I2C4_SDA
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);

    /* I2C4 interrupt DeInit */
    HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);
    /* USER CODE BEGIN I2C4_MspDeInit 1 */

    /* USER CODE END I2C4_MspDeInit 1 */
  }
}

void I2C4_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&hi2c4);
}

/**
  * @brief This function handles I2C4 error interrupt.
  */
void I2C4_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&hi2c4);
}
