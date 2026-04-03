#include "canbus.h"

#define FDCAN_NVIC_PRIO configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 5

FDCAN_HandleTypeDef* carfdcan;


can_status_t canbus_init(){
    carfdcan = hfdcan3;

    // carfdcan->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    // carfdcan->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    // carfdcan->Init.Mode = FDCAN_MODE_NORMAL;
    // carfdcan->Init.AutoRetransmission = DISABLE;
    // carfdcan->Init.TransmitPause = DISABLE;
    // carfdcan->Init.ProtocolException = DISABLE;
    // carfdcan->Init.NominalPrescaler = 20;
    // carfdcan->Init.NominalSyncJumpWidth = 1;
    // carfdcan->Init.NominalTimeSeg1 = 13;
    // carfdcan->Init.NominalTimeSeg2 = 2;
    // carfdcan->Init.DataPrescaler = 1;
    // carfdcan->Init.DataSyncJumpWidth = 1;
    // carfdcan->Init.DataTimeSeg1 = 1;
    // carfdcan->Init.DataTimeSeg2 = 1;
    // carfdcan->Init.StdFiltersNbr = 0;
    // carfdcan->Init.ExtFiltersNbr = 0;
    // carfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    // CubeMX gen'ed
    
    carfdcan->Instance = FDCAN3;
    carfdcan->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    carfdcan->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    carfdcan->Init.Mode = FDCAN_MODE_NORMAL;
    carfdcan->Init.AutoRetransmission = DISABLE;
    carfdcan->Init.TransmitPause = DISABLE;
    carfdcan->Init.ProtocolException = DISABLE;
    carfdcan->Init.NominalPrescaler = 16;
    carfdcan->Init.NominalSyncJumpWidth = 1;
    carfdcan->Init.NominalTimeSeg1 = 1;
    carfdcan->Init.NominalTimeSeg2 = 1;
    carfdcan->Init.DataPrescaler = 1;
    carfdcan->Init.DataSyncJumpWidth = 1;
    carfdcan->Init.DataTimeSeg1 = 1;
    carfdcan->Init.DataTimeSeg2 = 1;
    carfdcan->Init.StdFiltersNbr = 0;
    carfdcan->Init.ExtFiltersNbr = 0;
    carfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    // accepts all CAN IDs from 
    // FDCAN1 Filter Config
    FDCAN_FilterTypeDef CarCANFilterConfig;
    CarCANFilterConfig.IdType = FDCAN_STANDARD_ID;
    CarCANFilterConfig.FilterIndex = 0;
    CarCANFilterConfig.FilterType = FDCAN_FILTER_MASK;
    CarCANFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // directs frames to FIFO0
    CarCANFilterConfig.FilterID1 = 0x000;
    CarCANFilterConfig.FilterID2 = 0x000;

    if(can_fd_init(carfdcan, &CarCANFilterConfig) != CAN_OK){
       return CAN_ERR;
    }

    if(can_fd_start(carfdcan) != CAN_OK){
       return CAN_ERR;
    }

    return CAN_OK;
}

can_status_t canbus_send(uint32_t id, uint8_t dlc, uint8_t data[], TickType_t delay_ticks){
    if(carfdcan == NULL){
        return CAN_ERR;
    }

    FDCAN_TxHeaderTypeDef tx_header = {0};

    tx_header.Identifier = id;
    tx_header.IdType = FDCAN_STANDARD_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = dlc;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
    tx_header.MessageMarker = 0;

    return can_fd_send(carfdcan, &tx_header, data, delay_ticks);
}

can_status_t canbus_recv(uint32_t id, uint8_t data[], TickType_t delay_ticks){
    return CAN_ERR;
}


static uint32_t HAL_RCC_FDCAN_CLK_ENABLED=0;

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if(fdcanHandle->Instance==FDCAN1)
  {
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if(1){
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */ 
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, FDCAN_NVIC_PRIO, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
  }

  if(hfdcan->Instance==FDCAN3)
  {
    /* USER CODE BEGIN FDCAN3_MspInit 0 */

    /* USER CODE END FDCAN3_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN3 GPIO Configuration
    PA8     ------> FDCAN3_RX
    PA15     ------> FDCAN3_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF11_FDCAN3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN FDCAN3_MspInit 1 */

    /* USER CODE END FDCAN3_MspInit 1 */

  }

}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{
    if(fdcanHandle->Instance==FDCAN1)
    {
        /* Peripheral clock disable */
        HAL_RCC_FDCAN_CLK_ENABLED--;
        if(HAL_RCC_FDCAN_CLK_ENABLED==0){
        __HAL_RCC_FDCAN_CLK_DISABLE();
        }

        /**FDCAN1 GPIO Configuration
        PA11     ------> FDCAN1_RX
        PA12     ------> FDCAN1_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

        /* FDCAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
    }

    else if(fdcanHandle->Instance==FDCAN3)
    {
        /* Peripheral clock disable */
        HAL_RCC_FDCAN_CLK_ENABLED--;
        if(HAL_RCC_FDCAN_CLK_ENABLED==0){
        __HAL_RCC_FDCAN_CLK_DISABLE();
        }

        /**FDCAN3 GPIO Configuration
        PA8     ------> FDCAN3_RX
        PA15     ------> FDCAN3_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8|GPIO_PIN_15);

        /* FDCAN3 interrupt Deinit */
        HAL_NVIC_DisableIRQ(FDCAN3_IT0_IRQn); 
        HAL_NVIC_DisableIRQ(FDCAN3_IT1_IRQn);
    }
}