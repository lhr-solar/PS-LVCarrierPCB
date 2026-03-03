#include "bq25756e.h"

I2C_HandleTypeDef hi2c4;

void bq25756e_assert_bits(uint8_t* data, uint8_t bitstring);
void bq25756e_clear_bits(uint8_t* data, uint8_t bitstring);

void bq25756e_RevMode_disable(uint8_t* STAT, uint8_t buff1[]);
void bq25756e_TempSense_disable(uint8_t* STAT, uint8_t buff1[]);
void bq25756e_HiZ_disable(uint8_t* STAT, uint8_t buff1[]);
void bq25756e_SW_Ichg_enable(uint8_t *STAT, uint8_t buff1[], uint8_t buff2[]);
void bq25756e_HW_Ichg_disable(uint8_t* STAT, uint8_t buff[]);


uint8_t bq25756e_charge(bq25756e_message_t msg) {
  uint8_t STAT=0;
  // Dummy Data Buffers
  uint8_t pBuff1, pBuff2[1]; 

  if (msg == BQ25756E_CHRG_START) {
    // Disable Charge Limit
    bq25756e_HW_Ichg_disable(&STAT, pBuff1);
    // Write Charge Limit
    bq25756e_SW_Ichg_enable(&STAT, pBuff1, pBuff2);
    // Disable Hi Z
    bq25756e_HiZ_disable(&STAT, pBuff1);
    // Disable Temp Sense
    bq25756e_TempSense_disable(&STAT, pBuff1);
    // Disable Rev Mode 
    bq25756e_RevMode_disable(&STAT, pBuff1);

  } else if (msg == BQ25756E_PET_WDG) {
    // Pet watchdog
    bq25756e_pet_wdg(&STAT, pBuff1);

  } else if (msg == BQ25756E_CHRG_DUMP) {
    // Pretty print status to serial
    bq25756e_dump_status(&STAT, pBuff1);

  } else if (msg == BQ25756E_CHRG_STOP) {
    // Disable CE
    bq25756e_charge_disable(&STAT, pBuff1);
  }

  return STAT;
}

void bq25756e_charge_disable(uint8_t* STAT, uint8_t buff[]) {
  // Charge disable
  *STAT=bq25756e_read_reg(REG_CHARGE_CONTROL, buff);
  bq25756e_clear_bits(buff, BIT_CHARGE_ENABLE);
  *STAT=bq25756e_write_reg(REG_CHARGE_CONTROL, buff[0]);
}

void bq25756e_dump_status(uint8_t *STAT, uint8_t buff[]){
  // Dump charger status
  *STAT=bq25756e_read_reg(REG_CHARGE_STATUS_1, buff);
  printf("reg charge status 1: %d\n\r", buff[0]);
  *STAT=bq25756e_read_reg(REG_CHARGE_CONTROL, buff);
  printf("reg charge control: %d\n\r", buff[0]);
}

void bq25756e_RevMode_disable(uint8_t* STAT, uint8_t buff[]) {
  // Disable reverse mode (battery -> input)
  *STAT=bq25756e_read_reg(REG_REVERSE_MODE, buff);
  bq25756e_clear_bits(buff, BIT_REVERSE_MODE_ENABLE);
  *STAT=bq25756e_write_reg(REG_REVERSE_MODE, buff[0]);
}

void bq25756e_TempSense_disable(uint8_t* STAT, uint8_t buff[]) {
  // Disables temperature sense (HW temp sense is pulled low)
  *STAT=bq25756e_read_reg(REG_TEMP, buff);
  bq25756e_clear_bits(buff, BIT_TEMP_SENSE_ENABLE);
  *STAT=bq25756e_write_reg(REG_TEMP, buff[0]);
}

void bq25756e_HiZ_disable(uint8_t* STAT, uint8_t buff[]) {
  // Disables HiZ mode
  *STAT=bq25756e_read_reg(REG_PIN_CONTROL, buff);
  bq25756e_clear_bits(buff, BIT_HIZ_ENABLE);
  *STAT=bq25756e_write_reg(REG_PIN_CONTROL, buff[0]);
}

void bq25756e_SW_Ichg_enable(uint8_t *STAT, uint8_t buff1[], uint8_t buff2[]) {
  // Enables SW charge limit based on macros
  *STAT=bq25756e_read_reg(REG_CHARGE_CURRENT_LIMIT_B, buff1); // 0x06 
  *STAT=bq25756e_read_reg(REG_CHARGE_CURRENT_LIMIT_A, buff2); // 0x40 

  bq25756e_clear_bits(buff1, BIT_CHARGE_CURRENT_FIELD_A); 
  bq25756e_clear_bits(buff2, BIT_CHARGE_CURRENT_FIELD_B);
  bq25756e_assert_bits(buff1, BIT_CHARGE_CURRENT_MASK_A);
  bq25756e_assert_bits(buff2, BIT_CHARGE_CURRENT_MASK_B);

  *STAT=bq25756e_write_reg(REG_CHARGE_CURRENT_LIMIT_B, buff1[0]);  
  *STAT=bq25756e_write_reg(REG_CHARGE_CURRENT_LIMIT_A, buff2[0]); 
}

void bq25756e_HW_Ichg_disable(uint8_t* STAT, uint8_t buff[]) {
  // Disables hardware charge limit
  *STAT=bq25756e_read_reg(REG_PIN_CONTROL, buff);
  bq25756e_clear_bits(buff, BIT_CHARGE_LIMIT_ENABLE);
  *STAT=bq25756e_write_reg(REG_PIN_CONTROL, buff[0]);
}

void bq25756e_pet_wdg(uint8_t* STAT, uint8_t buff[]) {
  *STAT=bq25756e_read_reg(REG_CHARGE_CONTROL, buff);
  bq25756e_assert_bits(buff, BIT_WDG_RESET);
  *STAT=bq25756e_write_reg(REG_CHARGE_CONTROL, buff[0]);
}

void bq25756e_write_ce(uint8_t value) {
  // SW write to charge enable pin 
  HAL_GPIO_WritePin(BQ25756E_CE_PORT, BQ25756E_CE_PIN, value);
}

uint8_t bq25756e_read_reg(uint8_t reg, uint8_t* buffer) {
  uint8_t STAT;

  // TX
  STAT = HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), &reg, 1, HAL_MAX_DELAY);
  // RX
  STAT = HAL_I2C_Master_Receive(&hi2c4, (DEVICE_ADDR << 1), buffer, 1, HAL_MAX_DELAY);

  return STAT;
}

uint8_t bq25756e_write_reg(uint8_t reg, uint8_t data) {
  uint8_t STAT;

  // RX
  uint8_t payload[2];
  payload[0]=reg;
  payload[1]=data;
  STAT=HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), payload, 2, HAL_MAX_DELAY);

  return STAT;
}

void bq25756e_init(void) {     
    bq25756e_gpio_init();
    bq25756e_i2c4_init();
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
  gpio_clock_init(BQ25756E_CE_PORT);
  gpio_pin_init(BQ25756E_CE_PORT, BQ25756E_CE_PIN, OUTPUT_PP);
}

void bq25756e_i2c4_init(void)
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
    Error_Handler();
  }

  /** Configure Analog filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }

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

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}