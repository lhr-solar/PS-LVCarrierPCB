#include "ShiftRegister.h"

#include "stm32xx_hal.h"

static GPIO_InitTypeDef SR_configA = {.Mode = GPIO_MODE_OUTPUT_PP,
                                      .Pull = GPIO_NOPULL,
                                      .Speed = GPIO_SPEED_FREQ_LOW,
                                      .Pin = 0};
static GPIO_InitTypeDef SR_configB = {.Mode = GPIO_MODE_OUTPUT_PP,
                                      .Pull = GPIO_NOPULL,
                                      .Speed = GPIO_SPEED_FREQ_LOW,
                                      .Pin = 0};

const pin_t SR_pins[NUMBER_PINS] = {
    {GPIO_PIN_5, GPIOB},   // ser data
    {GPIO_PIN_10, GPIOA},  // srclk
    {GPIO_PIN_3, GPIOB}};  // status led need rclk

static const uint8_t seven_seg_map[10] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};

void ShiftRegister_Init() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  for (uint8_t i = 0; i < NUMBER_PINS; i++) {
    if (SR_pins[i].Port == GPIOA) {
      SR_configA.Pin |= SR_pins[i].Pin;
    } else if (SR_pins[i].Port == GPIOB) {
      SR_configB.Pin |= SR_pins[i].Pin;
    }
  }

  HAL_GPIO_Init(GPIOA, &SR_configA);
  HAL_GPIO_Init(GPIOB, &SR_configB);
}

void ShiftData_In(uint8_t data) {
  for (int i = 7; i >= 0; i--) {
    GPIO_PinState bit = (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(SR_pins[SR_SER].Port, SR_pins[SR_SER].Pin, bit);

    HAL_GPIO_WritePin(SR_pins[SR_SRCLK].Port, SR_pins[SR_SRCLK].Pin,
                      GPIO_PIN_SET);
    HAL_GPIO_WritePin(SR_pins[SR_SRCLK].Port, SR_pins[SR_SRCLK].Pin,
                      GPIO_PIN_RESET);
  }
}

void ShiftData_Latch() {
  HAL_GPIO_WritePin(SR_pins[SR_RCLK].Port, SR_pins[SR_RCLK].Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SR_pins[SR_RCLK].Port, SR_pins[SR_RCLK].Pin,
                    GPIO_PIN_RESET);
}

// displays
void DisplayData_SevenSeg(uint8_t d0, uint8_t d1, uint8_t d2) {
  uint8_t seg0 = ~seven_seg_map[d0];
  uint8_t seg1 = ~(seven_seg_map[d1] | 0b10000000);  // decimal
  uint8_t seg2 = ~seven_seg_map[d2];

  ShiftData_In(seg2);
  ShiftData_In(seg1);
  ShiftData_In(seg0);

  ShiftData_Latch();
}

// SRCLR is tied to GND in hardware, so no clear function.