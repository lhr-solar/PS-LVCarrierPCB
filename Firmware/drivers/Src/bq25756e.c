#include "bq25756e.h"

// Driver's BQ Handle
static BQ_HandleTypeDef bq_handle_repr;
static BQ_HandleTypeDef* bq_handle = &bq_handle_repr;

// I2C Bus Semaphore Buffer
static StaticSemaphore_t xSemaphoreBuffer;

// Size of bus payloads
#define BQ_RX_SIZE 1
#define BQ_TX_SIZE 2

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
static bq25756e_status_t bq25756e_i2c_init(void);

bq25756e_status_t bq25756e_charge(TickType_t delay, bq25756e_chg_current_t limit) {
    // Charge Function
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
    // Assert CE Pin to start charging
    bq25756e_write_ce(BQ25756E_LOGIC_HIGH);

    return stat;
}

bq25756e_status_t bq25756e_pet_wdg(TickType_t delay) {
  uint8_t buff[1]={0};

  // Read charge control register [0x17]
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  // Friendly write 1 to WDG_RST (resets watchdog timer)
  bq25756e_assert_bits(buff, BQ25756E_BIT_WDG_RESET);
  // Write back to control register
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_dump_status(bq25756e_charge_status_t *charge_state,
                                       bq25756e_serial_config_t serial, 
                                       TickType_t delay ) {
  uint8_t buff[1]={0};

  // Dump charger status
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_STATUS_1, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  
  if (serial==BQ25756E_SERIAL_ENABLE) printf("Charge Status: ");
  uint8_t charge_stat = buff[0] & BQ25756E_BIT_CHARGE_STAT;
  switch (charge_stat) {
    case 0:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Not charging. \n\r"); 
      *charge_state = BQ25756E_NOT_CHRG; 
      break;
    case 1:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Trickle Charge. \n\r"); 
      *charge_state = BQ25756E_TRICKLE;
      break;
    case 2:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Pre-Charge. \n\r"); 
      *charge_state = BQ25756E_PRE;
      break;
    case 3:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Fast Charge. \n\r"); 
      *charge_state = BQ25756E_FAST;
      break;
    case 4:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Taper Charge. \n\r"); 
      *charge_state = BQ25756E_TAPER;
      break;
    case 5:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("y is it here lmao ur cooked. \n\r"); 
      *charge_state = BQ25756E_RESERVED;
      break;
    case 6:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Top off Timer Charge. \n\r"); 
      *charge_state = BQ25756E_TOP_OFF;
      break;
    case 7:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Charge Termination Done. \n\r"); 
      *charge_state = BQ25756E_DONE_CHRG;
      break;
    default: break;
  }

  // shouldn't hit this --- invalid status
  return BQ25756E_ERR;
}

bq25756e_status_t bq25756e_dump_faults(uint8_t *fault_state,
                                       bq25756e_serial_config_t serial, 
                                       TickType_t delay ) {
  uint8_t buff[1]={0};
  uint8_t fault_cnt=0;

  if (bq25756e_read_reg(BQ25756E_REG_FAULT_STATUS, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }

  if (serial==BQ25756E_SERIAL_ENABLE) printf("Faults: \n-------\n");
  if (buff[0] & BQ25756E_BIT_INPUT_UV_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Input Undervoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_INPUT_OV_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Input Overvoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_BATT_OVERCURRENT_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Battery Overcurrent! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_BATT_OV_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Battery Overvoltage! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_THERMAL_SHDN_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Thermal Shutdown! \n");
    fault_cnt++;
  }
  if (buff[0] & BQ25756E_BIT_DRV_SUP_FAULT) {
    if (serial==BQ25756E_SERIAL_ENABLE) printf("Gate supply drive out of range! \n");
    fault_cnt++;
  }
  if (serial==BQ25756E_SERIAL_ENABLE) printf("Num Faults: %d\n-------\n\r", fault_cnt);

  *fault_state = buff[0]; // write back raw fault register contents

  if (fault_cnt > 0) return BQ25756E_ERR;
  else return BQ25756E_OK;

}

bq25756e_status_t bq25756e_charge_disable(TickType_t delay) {
  uint8_t buff[1]={0};

  // Charge disable
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CONTROL, buff, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BQ25756E_BIT_CHARGE_ENABLE);
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CONTROL, buff[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  // Disable CE
  bq25756e_write_ce(BQ25756E_LOGIC_LOW);

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_init(BQ_HandleTypeDef *_bq_handle, I2C_HandleTypeDef *bq_i2c_handle) {   
  bq25756e_status_t stat = BQ25756E_OK;
  bq25756e_gpio_init();
  
  _bq_handle->hi2c = bq_i2c_handle;
  _bq_handle->bq_i2c_smphr = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );
  _bq_handle->device_id = DEVICE_ADDR << 1;

  if (_bq_handle->bq_i2c_smphr == NULL) return BQ25756E_ERR;
  
  // Set global in driver to point to newly created handle
  bq_handle=_bq_handle;

  bq25756e_i2c_init();

  // init pre req bits
  bq25756e_preReqBits_init();

  // Start in disabled state to be safeeeee
  bq25756e_write_ce(BQ25756E_LOGIC_LOW);

  return stat;
}

void bq25756e_write_ce(bq25756e_logic_t value) {
  // SW write to charge enable pin 
  HAL_GPIO_WritePin(BQ25756E_CE_PORT, BQ25756E_CE_PIN, value == BQ25756E_LOGIC_HIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/****************************** STATIC DRIVER HELPERS ********************************/

static bq25756e_status_t bq25756e_RevMode_disable(TickType_t delay) {
  // Disable reverse mode (battery -> input)
  uint8_t buff[1]={0};
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
  uint8_t buff[1]={0};
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
  uint8_t buff[1]={0};
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
  uint8_t buff1[1], buff2[1] = {0};

  /**
   *  The software charge limit should ALWAYS be set.
   * 
   *  Registers:
   *  - 0x03[15:8] --> TOP 8 bits of charge limit
   *  - 0x02[7:0]  --> BOTTOM 8 bits of charge limit
   * 
   *  Masks were precalculated for common limit values w/ type `bq25756e_chg_current_t`
   */

  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_B, buff2, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x03
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_A, buff1, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x02 

  bq25756e_clear_bits(buff1, BQ25756E_BIT_CHARGE_CURRENT_FIELD_A); 
  bq25756e_clear_bits(buff2, BQ25756E_BIT_CHARGE_CURRENT_FIELD_B);

  uint16_t mask;
  uint8_t mask_b, mask_a; // split among 2 registers 0x03 and 0x02

  mask=limit;
  
  // Shift 0x03 register mask to the top byte position [15:8]
  mask_b=(uint8_t) (mask >> 0x08);
  // Only get bottom byte of 0x02 register mask [7:0]
  mask_a=(uint8_t) (mask & 0xFF);
  bq25756e_assert_bits(buff1, mask_a);
  bq25756e_assert_bits(buff2, mask_b);

  // Write back to limit registers
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_B, buff2[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }  
  if (bq25756e_write_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_A, buff1[0], delay) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_HW_Ichg_disable(TickType_t delay) {
  // Disables hardware charge limit
  uint8_t buff[1]={0};
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
  if ( HAL_I2C_Master_Transmit_IT(bq_handle->hi2c, (bq_handle->device_id), &reg, BQ_RX_SIZE) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }
  if (xSemaphoreTake(bq_handle->bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  // RX
  if ( HAL_I2C_Master_Receive_IT(bq_handle->hi2c, (bq_handle->device_id), buffer, BQ_RX_SIZE) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }
  if (xSemaphoreTake(bq_handle->bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  return BQ25756E_OK;
}

static bq25756e_status_t bq25756e_write_reg(uint8_t reg, uint8_t data, TickType_t delay) {
  // TX
  uint8_t payload[BQ_TX_SIZE];
  payload[0]=reg;
  payload[1]=data;

  if (HAL_I2C_Master_Transmit_IT (bq_handle->hi2c, (bq_handle->device_id), payload, BQ_TX_SIZE) != HAL_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  if (xSemaphoreTake(bq_handle->bq_i2c_smphr, pdMS_TO_TICKS(delay)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }

  return BQ25756E_OK;
}

static void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  &  ~(bitstring));
}

static void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  |  (bitstring));
}

static void bq25756e_gpio_init(void)
{
  // Init Charge Enable pin
  gpio_pin_init(BQ25756E_CE_PORT, BQ25756E_CE_PIN, OUTPUT);
  // Disable CE pin on startup
  bq25756e_write_ce(BQ25756E_LOGIC_LOW);  
  gpio_pin_init(BQ25756E_STAT2_PORT, BQ25756E_STAT2_PIN, INPUT);
  gpio_pin_init(BQ25756E_STAT1_PORT, BQ25756E_STAT1_PIN, INPUT);
  gpio_pin_init(BQ25756E_INT_PORT, BQ25756E_INT_PIN, INPUT);
}

/****************************** I2C INITS / DEINITS ********************************/

static bq25756e_status_t bq25756e_i2c_init(void)
{
  bq_handle->hi2c->Instance = BQ25756E_I2C_PERIPH;
  bq_handle->hi2c->Init.Timing = 0x00503D58;
  bq_handle->hi2c->Init.OwnAddress1 = 0;
  bq_handle->hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  bq_handle->hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  bq_handle->hi2c->Init.OwnAddress2 = 0;
  bq_handle->hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  bq_handle->hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  bq_handle->hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(bq_handle->hi2c) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  /** Configure Analog filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(bq_handle->hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(bq_handle->hi2c, 0) != HAL_OK)
  {
    return BQ25756E_INIT_FAIL;
  }

  return BQ25756E_OK;
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(hi2c->Instance==BQ25756E_I2C_PERIPH)
  {
    /* USER CODE BEGIN I2C4_MspInit 0 */

    /* USER CODE END I2C4_MspInit 0 */

  /** Initializes the peripherals clocks
  */

    #ifdef I2C1
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    #endif
    #ifdef I2C2
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    #endif
    #ifdef I2C3
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    #endif
    #ifdef I2C5
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
    PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    #endif

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }
    
    if (BQ25756E_I2C_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (BQ25756E_I2C_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (BQ25756E_I2C_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = BQ25756E_I2C_SDA_PIN|BQ25756E_I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = BQ25756E_I2C_AF;
    HAL_GPIO_Init(BQ25756E_I2C_PORT, &GPIO_InitStruct);

    /* Peripheral clock enable */
    if (hi2c->Instance == I2C1) __HAL_RCC_I2C1_CLK_ENABLE();
    else if (hi2c->Instance == I2C2) __HAL_RCC_I2C2_CLK_ENABLE();
    else if (hi2c->Instance == I2C3) __HAL_RCC_I2C3_CLK_ENABLE();
    else if (hi2c->Instance == I2C4) __HAL_RCC_I2C4_CLK_ENABLE();
    
    /* I2C4 interrupt Init */
    #ifdef I2C1_NVIC_PRIO
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C1_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C1_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    #endif

    #ifdef I2C2_NVIC_PRIO
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, I2C2_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, I2C2_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    #endif

    #ifdef I2C3_NVIC_PRIO
    HAL_NVIC_SetPriority(I2C3_EV_IRQn, I2C3_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_SetPriority(I2C3_ER_IRQn, I2C3_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
    #endif

    #ifdef I2C4_NVIC_PRIO
    HAL_NVIC_SetPriority(I2C4_EV_IRQn, I2C4_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_SetPriority(I2C4_ER_IRQn, I2C4_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
    #endif
    /* USER CODE BEGIN I2C4_MspInit 1 */

    /* USER CODE END I2C4_MspInit 1 */

  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==BQ25756E_I2C_PERIPH)
  {
    /* USER CODE BEGIN I2C4_MspDeInit 0 */

    /* USER CODE END I2C4_MspDeInit 0 */
    /* Peripheral clock disable */
    if (hi2c->Instance == I2C1) __HAL_RCC_I2C1_CLK_DISABLE();
    if (hi2c->Instance == I2C2) __HAL_RCC_I2C2_CLK_DISABLE();
    if (hi2c->Instance == I2C3) __HAL_RCC_I2C3_CLK_DISABLE();
    if (hi2c->Instance == I2C4) __HAL_RCC_I2C4_CLK_DISABLE();
    
    HAL_GPIO_DeInit(BQ25756E_I2C_PORT, BQ25756E_I2C_SCL_PIN);

    HAL_GPIO_DeInit(BQ25756E_I2C_PORT, BQ25756E_I2C_SDA_PIN);

    /* I2C4 interrupt DeInit */
    #ifdef I2C1_NVIC_PRIO
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
    #endif
    #ifdef I2C2_NVIC_PRIO
    HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    #endif
    #ifdef I2C3_NVIC_PRIO
    HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    #endif
    #ifdef I2C4_NVIC_PRIO
    HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);
    #endif
    /* USER CODE BEGIN I2C4_MspDeInit 1 */

    /* USER CODE END I2C4_MspDeInit 1 */
  }
}

/****************************** I2C CALLBACKS ********************************/

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  xSemaphoreGiveFromISR(bq_handle->bq_i2c_smphr, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  xSemaphoreGiveFromISR(bq_handle->bq_i2c_smphr, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  // kinda scuffed cuz if NAK -> hits this callback so still release

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  xSemaphoreGiveFromISR(bq_handle->bq_i2c_smphr, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void I2C1_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(bq_handle->hi2c);
}
/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(bq_handle->hi2c);
}

#ifdef I2C2
void I2C2_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(bq_handle->hi2c);
}

/**
  * @brief This function handles I2C2 error interrupt.
  */
void I2C2_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(bq_handle->hi2c);
}
#endif

#ifdef I2C3
void I2C3_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(bq_handle->hi2c);
}

/**
  * @brief This function handles I2C3 error interrupt.
  */
void I2C3_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(bq_handle->hi2c);
}
#endif

#ifdef I2C4
void I2C4_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(bq_handle->hi2c);
}

/**
  * @brief This function handles I2C4 error interrupt.
  */
void I2C4_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(bq_handle->hi2c);
}
#endif