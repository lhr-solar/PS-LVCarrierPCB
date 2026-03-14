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
    case 1:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Trickle Charge. \n\r"); 
      *charge_state = BQ25756E_TRICKLE;
    case 2:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Pre-Charge. \n\r"); 
      *charge_state = BQ25756E_PRE;
    case 3:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Fast Charge. \n\r"); 
      *charge_state = BQ25756E_FAST;
    case 4:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Taper Charge. \n\r"); 
      *charge_state = BQ25756E_TAPER;
    case 5:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("y is it here lmao ur cooked. \n\r"); 
      *charge_state = BQ25756E_RESERVED;
    case 6:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Top off Timer Charge. \n\r"); 
      *charge_state = BQ25756E_TOP_OFF;
    case 7:
      if (serial==BQ25756E_SERIAL_ENABLE) printf("Charge Termination Done. \n\r"); 
      *charge_state = BQ25756E_DONE_CHRG;
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

  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_B, buff1, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x03
  if (bq25756e_read_reg(BQ25756E_REG_CHARGE_CURRENT_LIMIT_A, buff2, delay) != BQ25756E_OK) {
    return BQ25756E_READ_FAIL;
  } // 0x02 

  bq25756e_clear_bits(buff1, BQ25756E_BIT_CHARGE_CURRENT_FIELD_A); 
  bq25756e_clear_bits(buff2, BQ25756E_BIT_CHARGE_CURRENT_FIELD_B);

  uint8_t mask, mask_b, mask_a; // split among 2 registers 0x03 and 0x02

  mask=limit;
  
  // Shift 0x03 register mask to the top byte position [15:8]
  mask_b=(uint8_t) (mask >> 0x08);
  // Only get bottom byte of 0x02 register mask [7:0]
  mask_a=(uint8_t) (mask & 0xFF);
  bq25756e_assert_bits(buff1, mask_a);
  bq25756e_assert_bits(buff2, mask_b);

  // Write back to limit registers
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
  // kinda scuffed cuz if NAK hits this callback so still release

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  xSemaphoreGiveFromISR(bq_handle->bq_i2c_smphr, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


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
