#include "stm32xx_hal.h"
#include "bq25756e.h"
#include "statusLeds.h"
#include "commandLine.h"
#include "ltc4421.h"
#include "pinDefs.h"
#include "faultBits.h"

#define MS_DELAY_100 pdMS_TO_TICKS(100) 
#define MS_DELAY_500 pdMS_TO_TICKS(500) 

// Greater priority than default RTOS interrupt
#define I2C4_NVIC_PRIO  configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1

// Run test w/ arbitrary fault after ~10s
#define FAULT_TEST

/* BQ Driver Setup */
// User's BQ Handle
static BQ_HandleTypeDef bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c4;

// Task buffers
StaticTask_t bqTaskBuffer;
StackType_t bqTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t faultTaskBuffer;
StackType_t faultTaskStack[configMINIMAL_STACK_SIZE];

bq25756e_status_t i2c_init(void);

void BqTask(void *argument){
    faultBits_init();
    // give chip a bit to power on
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    bq25756e_preReqBit_wait(BQ25756E_PREREQ_LTC_VALID, portMAX_DELAY);

    statusLeds_toggle(LSOM_HEARTBEAT_LED);

    bq25756e_charge_status_t *charge_state=BQ25756E_NOT_CHRG;
    bq25756e_charge(portMAX_DELAY, BQ25756E_1A);

    while (1) {
        statusLeds_toggle(LSOM_HEARTBEAT_LED);

        #ifdef FAULT_TEST
        // Only run fault test
        if (faultBit_wait(FAULT_SUPPREG_UNDERVOLTAGE, pdMS_TO_TICKS(200)) != pdFALSE) {
            bq25756e_charge(portMAX_DELAY, BQ25756E_1A);
            vTaskDelete(NULL);
        }  
        #endif

        bq25756e_charge(portMAX_DELAY, BQ25756E_1A);

        // // Dump status and continue
        bq25756e_dump_status(charge_state, BQ25756E_SERIAL_ENABLE, portMAX_DELAY); 
        bq25756e_pet_wdg(portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}   


void FaultTask(void *argument){
    // Signal LTC and supp good
    vTaskDelay(pdMS_TO_TICKS(1000)); 
    bq25756e_set_preReqBit(BQ25756E_PREREQ_LTC_VALID);

    // Trip fault after 10s
    vTaskDelay(pdMS_TO_TICKS(10000)); 
    // Throw a random fault
    while (1) {  
        vTaskDelay(pdMS_TO_TICKS(500));
        set_faultBit(FAULT_SUPPREG_UNDERVOLTAGE);      
    }
}

int main()
{
    HAL_Init();
    i2c_init();
    SystemClock_Config();

    statusLeds_init();
    bq25756e_init(&bq_handle, &hi2c4);
    command_line_init();
    
    // Start in disabled state to be safeeeee
    bq25756e_write_ce(BQ25756E_LOGIC_LOW);

    xTaskCreateStatic(BqTask, 
                     "BQ test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     bqTaskStack,
                     &bqTaskBuffer);


    xTaskCreateStatic(FaultTask, 
                     "Fault test",
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     tskIDLE_PRIORITY + 2,
                     faultTaskStack,
                     &faultTaskBuffer);

    vTaskStartScheduler();
    
    while (1)
    {
        // should never be here
    }
}

bq25756e_status_t i2c_init(void)
{
  /* Using I2C4 */

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
    HAL_NVIC_SetPriority(I2C4_EV_IRQn, I2C4_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);
    HAL_NVIC_SetPriority(I2C4_ER_IRQn, I2C4_NVIC_PRIO, 0);
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