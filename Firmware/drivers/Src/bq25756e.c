#include "bq25756e.h"

I2C_HandleTypeDef hi2c4;

// I2C Bus Semaphore
SemaphoreHandle_t control = NULL;
static StaticSemaphore_t xSemaphoreBuffer;

void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring);
void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring);

bq25756e_status_t bq25756e_RevMode_disable(uint8_t buff1[]);
bq25756e_status_t bq25756e_TempSense_disable(uint8_t buff1[]);
bq25756e_status_t bq25756e_HiZ_disable(uint8_t buff1[]);
bq25756e_status_t bq25756e_SW_Ichg_enable(uint8_t buff1[], uint8_t buff2[]);
bq25756e_status_t bq25756e_HW_Ichg_disable(uint8_t buff[]);
bq25756e_status_t bq25756e_dump_status(uint8_t buff[]);
bq25756e_status_t bq25756e_charge_disable(uint8_t buff[]);

bq25756e_status_t bq25756e_charge(bq25756e_message_t msg) {
  bq25756e_status_t STAT=BQ25756E_OK;
  // Dummy Data Buffers
  uint8_t pBuff1[1], pBuff2[1]; 

  if (msg == BQ25756E_CHRG_START) {
    // Disable Charge Limit
    printf("hi1!\n\r");
    STAT=bq25756e_HW_Ichg_disable(pBuff1);
    // Write Charge Limit
    printf("hi2!\n\r");
    STAT=bq25756e_SW_Ichg_enable(pBuff1, pBuff2);
    // Disable Hi Z
    printf("hi3!\n\r");
    STAT=bq25756e_HiZ_disable(pBuff1);
    // Disable Temp Sense
    STAT=bq25756e_TempSense_disable(pBuff1);
    // Disable Rev Mode 
    STAT=bq25756e_RevMode_disable(pBuff1);
    // Pull CE Pin low to start charging
    bq25756e_write_ce(BQ25756E_LOGIC_LOW);

  } else if (msg == BQ25756E_PET_WDG) {
    // Pet watchdog
    STAT=bq25756e_pet_wdg(pBuff1);

  } else if (msg == BQ25756E_CHRG_DUMP) {
    // Pretty print status to serial
    STAT=bq25756e_dump_status(pBuff1);

  } else if (msg == BQ25756E_CHRG_STOP) {
    // Disable CE
    STAT=bq25756e_charge_disable(pBuff1);
    bq25756e_write_ce(BQ25756E_LOGIC_HIGH);
  }

  return STAT;
}

bq25756e_status_t bq25756e_charge_disable(uint8_t buff[]) {
  // Charge disable
  if (bq25756e_read_reg(REG_CHARGE_CONTROL, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BIT_CHARGE_ENABLE);
  if (bq25756e_write_reg(REG_CHARGE_CONTROL, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_dump_status(uint8_t buff[]){
  // Dump charger status
  if (bq25756e_read_reg(REG_CHARGE_STATUS_1, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  
  printf("Charge Status: ");
  uint8_t charge_stat = buff[0] & BIT_CHARGE_STAT;
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

bq25756e_status_t bq25756e_RevMode_disable(uint8_t buff[]) {
  // Disable reverse mode (battery -> input)
  if (bq25756e_read_reg(REG_REVERSE_MODE, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BIT_REVERSE_MODE_ENABLE);
  if (bq25756e_write_reg(REG_REVERSE_MODE, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  } 

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_TempSense_disable(uint8_t buff[]) {
  // Disables temperature sense (HW temp sense is pulled low)
  if (bq25756e_read_reg(REG_TEMP, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BIT_TEMP_SENSE_ENABLE);
  if (bq25756e_write_reg(REG_TEMP, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_HiZ_disable(uint8_t buff[]) {
  // Disables HiZ mode
  if (bq25756e_read_reg(REG_PIN_CONTROL, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BIT_HIZ_ENABLE);
  if (bq25756e_write_reg(REG_PIN_CONTROL, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_SW_Ichg_enable(uint8_t buff1[], uint8_t buff2[]) {
  // Enables SW charge limit based on macros
  if (bq25756e_read_reg(REG_CHARGE_CURRENT_LIMIT_B, buff1) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x06 
  if (bq25756e_read_reg(REG_CHARGE_CURRENT_LIMIT_A, buff2) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x40 

  bq25756e_clear_bits(buff1, BIT_CHARGE_CURRENT_FIELD_A); 
  bq25756e_clear_bits(buff2, BIT_CHARGE_CURRENT_FIELD_B);

  uint8_t mask, mask_b, mask_a; // split among 2 registers 0x03 and 0x02

  #ifdef CHG_LIM_1
  mask=BQ25756E_1A_MASK;
  #elif CHG_LIM_2_6
  mask=BQ25756E_2_6A_MASK;
  #elif CHG_LIM_5
  mask=BQ25756E_5A_MASK;
  #else
  mask=0; // limit unintended behavior  
  #endif
  
  mask_b=(uint8_t) (mask >> 0x08);
  mask_a=(uint8_t) (mask & 0xFF);
  bq25756e_assert_bits(buff1, mask_a);
  bq25756e_assert_bits(buff2, mask_b);

  if (bq25756e_write_reg(REG_CHARGE_CURRENT_LIMIT_B, buff1[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }  
  if (bq25756e_write_reg(REG_CHARGE_CURRENT_LIMIT_A, buff2[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_HW_Ichg_disable(uint8_t buff[]) {
  // Disables hardware charge limit
  if (bq25756e_read_reg(REG_PIN_CONTROL, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_clear_bits(buff, BIT_CHARGE_LIMIT_ENABLE);
  if (bq25756e_write_reg(REG_PIN_CONTROL, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_pet_wdg(uint8_t buff[]) {
  if (bq25756e_read_reg(REG_CHARGE_CONTROL, buff) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  }
  bq25756e_assert_bits(buff, BIT_WDG_RESET);
  if (bq25756e_write_reg(REG_CHARGE_CONTROL, buff[0]) != BQ25756E_OK) {
    return BQ25756E_WRITE_FAIL;
  }
  return BQ25756E_OK;
}

void bq25756e_write_ce(bq25756e_logic_t value) {
  // SW write to charge enable pin 
  HAL_GPIO_WritePin(BQ25756E_CE_PORT, BQ25756E_CE_PIN, value);
}

bq25756e_status_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer) {
  // TX
  if (xSemaphoreTake(control, pdMS_TO_TICKS(portMAX_DELAY)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }
  if ( HAL_I2C_Master_Transmit_IT(&hi2c4, (DEVICE_ADDR << 1), &reg, 1) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }

  printf("one semaphore taken!\n\r");

  // RX
  if (xSemaphoreTake(control, pdMS_TO_TICKS(portMAX_DELAY)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }
  if ( HAL_I2C_Master_Receive_IT(&hi2c4, (DEVICE_ADDR << 1), buffer, 1) != HAL_OK) {
    return BQ25756E_READ_FAIL;
  }
  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_write_reg(uint8_t reg, uint8_t data) {
  // RX
  uint8_t payload[2];
  payload[0]=reg;
  payload[1]=data;

  if (xSemaphoreTake(control, pdMS_TO_TICKS(portMAX_DELAY)) != pdTRUE) {
      return BQ25756E_TIMEOUT;
  }
  if (HAL_I2C_Master_Transmit_IT (&hi2c4, (DEVICE_ADDR << 1), payload, 2) != HAL_OK) {
    return BQ25756E_WRITE_FAIL;
  }

  return BQ25756E_OK;
}

bq25756e_status_t bq25756e_init(void) {   
  bq25756e_status_t STAT;
  bq25756e_gpio_init();
  STAT=bq25756e_i2c4_init();

  // Init semaphore
  control = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );
  if (control != NULL) xSemaphoreGive( control ); // open for grabs

  return STAT;
}

void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  &  ~(bitstring));
}

void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring) {
  *data = (*data  |  (bitstring));
}

void bq25756e_gpio_init(void)
{
  // Init Charge Enable pin
  gpio_pin_init(BQ25756E_CE_PORT, BQ25756E_CE_PIN, OUTPUT);
  gpio_pin_init(BQ25756E_STAT2_PORT, BQ25756E_STAT2_PIN, INPUT);
  gpio_pin_init(BQ25756E_STAT1_PORT, BQ25756E_STAT1_PIN, INPUT);
  gpio_pin_init(BQ25756E_INT_PORT, BQ25756E_INT_PIN, INPUT);
}

bq25756e_status_t bq25756e_i2c4_init(void)
{
  HAL_NVIC_SetPriority(I2C4_EV_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
  HAL_NVIC_SetPriority(I2C4_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);

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

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    printf("TX!\n\r");
    
    xSemaphoreGiveFromISR(control, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    printf("TX!\n\r");
    
    xSemaphoreGiveFromISR(control, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    printf("rip!\n\r");

    // todo
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle->Instance==I2C4)
  {
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
    GPIO_InitStruct.Pull = GPIO_PULLUP; // GPIO_NOPULL
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_I2C4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2C4 clock enable */
    __HAL_RCC_I2C4_CLK_ENABLE();
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C4)
  {
    /* Peripheral clock disable */
    __HAL_RCC_I2C4_CLK_DISABLE();

    /**I2C4 GPIO Configuration
    PC6     ------> I2C4_SCL
    PC7     ------> I2C4_SDA
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);
  }
}
