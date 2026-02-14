#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include "stm32xx_hal.h"

#define NUMBER_PINS 3
#define NUMBER_DISPLAYS 3
#define MAX_NUM 999  // The display will cap out display at 99.9 V.
#define NUM_DIGITS 10

typedef struct {
  uint16_t Pin;
  GPIO_TypeDef *Port;
} pin_t;

typedef enum { DISPLAY_0 = 0, DISPLAY_1 = 1, DISPLAY_2 = 2 } display_t;

typedef enum { SR_SER = 0, SR_SRCLK = 1, SR_RCLK = 2 } pins_t;

// Initializes the shift register pins, SER, SRCLK, and RCLK
/**
 * @brief   Initializes GPIO pins used to control the shift register
 *          (SER, SRCLK, and RCLK)
 * @param   None
 * @return  None
 */
void ShiftRegister_Init(void);

/**
 * @brief   Shifts one byte of data into the shift register (MSB first)
 * @param   data Byte to be shifted into the register
 * @return  None
 */
void ShiftData_In(uint8_t data);

/**
 * @brief   Latches the shifted data to the shift register outputs
 * @param   None
 * @return  None
 */
void ShiftData_Latch(void);

/**
 * @brief   Displays three digits on the seven-segment display using the shift
 * register
 * @param   d0 Digit to display on the leftmost display (0–9)
 * @param   d1 Digit to display on the middle display (0–9, decimal point
 * enabled)
 * @param   d2 Digit to display on the rightmost display (0–9)
 * @return  None
 */
void DisplayData_SevenSeg(uint8_t d0, uint8_t d1, uint8_t d2);

#endif  // SHIFTREGISTER_H
