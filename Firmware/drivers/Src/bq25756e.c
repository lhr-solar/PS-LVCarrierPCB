#include "bq25756e.h"

// SDA --- I2C4 
// SCL --- I2C4 

// Device Addr --- 0x6A
#define DEVICE_ADDR 0x6a

I2C_HandleTypeDef hi2c4;


void bq25756e_init() {     
    i2c4_init();
}

void bq25756e_transmit(message_t message) {  
    if (message == START) {    
        uint8_t stat;
        
        // uint8_t default_addr=0x0; 
        // data = pack(default_addr, 0);
        // stat = HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), &data, 1, HAL_MAX_DELAY); 

        uint8_t buffer[1];

        uint8_t charge_control_reg=0x17;
        
        // READ CHARGE CONTROL REG
        // write to device & register
        stat = HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), &charge_control_reg, 1, HAL_MAX_DELAY); 
        // HAL_Delay(100);
        // sent read request
        stat = HAL_I2C_Master_Receive(&hi2c4, (DEVICE_ADDR << 1), buffer, 1, HAL_MAX_DELAY); 

        HAL_Delay(500);

        // turn off charge enable
        // uint8_t data[2]; 
        // data[0]=charge_control_reg;
        // data[1]=(buffer[0] & 0b11111110);

        // // WRITE TO CHARGE CONTROL REG
        // stat = HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), data, 2, HAL_MAX_DELAY); 

        // HAL_Delay(500);

        // stat = HAL_I2C_Master_Transmit(&hi2c4, (DEVICE_ADDR << 1), &charge_control_reg, 1, HAL_MAX_DELAY); 
        // // HAL_Delay(100);
        // // sent read request
        // stat = HAL_I2C_Master_Receive(&hi2c4, (DEVICE_ADDR << 1), buffer, 1, HAL_MAX_DELAY); 

        (void)stat;
        // if (stat != HAL_OK) {
        //     Error_Handler();
        // }
    }
}

void i2c4_init(void)
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
    GPIO_InitStruct.Pull = GPIO_NOPULL;
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