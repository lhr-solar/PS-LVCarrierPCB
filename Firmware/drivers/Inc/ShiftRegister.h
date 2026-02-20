#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include "stm32xx_hal.h"

#define NUMBER_PINS 3
#define NUMBER_DISPLAYS 3
#define MAX_NUM 999 // The display will cap out display at 99.9 V.

#define SR_SER 0
#define SR_SRCLK 1
#define SR_RCLK 2

typedef struct {
  uint32_t Pin;
  GPIO_TypeDef *Port;
} pin_t;

enum display { display_0 = 0, display_1 = 1, display_2 = 2 };

typedef enum display display_t;

// Initializes the shift register pins, SER, SRCLK, and RCLK
void ShiftRegister_Init(void);

void ShiftData_In(uint32_t data);

void ShiftData_Latch(void);

void DisplayData_SevenSeg(uint8_t d0, uint8_t d1, uint8_t d2);

#endif // SHIFTREGISTER_H
