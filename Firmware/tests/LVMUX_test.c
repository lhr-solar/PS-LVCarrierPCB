#include "stm32xx_hal.h"

// --- 1. PIN DEFINITIONS (Clear & Configurable) ---
// All requested inputs are on GPIOB
#define INPUT_PORT      GPIOB

#define PIN_CHL2        GPIO_PIN_12
#define PIN_FLT1        GPIO_PIN_13
#define PIN_FLT2        GPIO_PIN_14
#define PIN_VDL1        GPIO_PIN_15
#define PIN_VDL2        GPIO_PIN_9 
// --- 2. DATA STRUCTURE (For Testability) ---
// By storing values here, you can inspect 'current_state' in your debugger
typedef struct {
    uint8_t chl2;
    uint8_t flt1;
    uint8_t flt2;
    uint8_t vdl1;
    uint8_t vdl2;
} InputState_t;

InputState_t current_state;
// --- EXISTING LED LOGIC ---
#ifdef STM32L432xx
    #define LED_PIN GPIO_PIN_3
    #define LED_PORT GPIOB
#elif STM32L431xx
    #define LED_PIN GPIO_PIN_11
    #define LED_PORT GPIOB
#else
    #define LED_PIN GPIO_PIN_5
    #define LED_PORT GPIOA
#endif
// Initialize clock for heartbeat LED port
void Heartbeat_Clock_Init() {
    if (LED_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (LED_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (LED_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
}
// --- 3. INPUT INITIALIZATION (Concise) ---
void Init_System_Inputs(void) {
    // 1. Enable Clock for Port B (Where all your inputs are)
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. Configure GPIOs
    GPIO_InitTypeDef init_config = {0};
    
    // We can OR (|) these together because they share the same settings
    init_config.Pin = PIN_CHL2 | PIN_FLT1 | PIN_FLT2 | PIN_VDL1 | PIN_VDL2;
    init_config.Mode = GPIO_MODE_INPUT;
    
    // NOTE: Use GPIO_PULLUP if signals are Open-Drain/Active-Low and lack external resistors
    // Use GPIO_NOPULL if the external hardware drives the signal High and Low actively.
    init_config.Pull = GPIO_NOPULL; 
    
    HAL_GPIO_Init(INPUT_PORT, &init_config);
}
// --- 4. HELPER FUNCTION ---
void Read_All_Inputs(InputState_t *state) {
    state->chl2 = HAL_GPIO_ReadPin(INPUT_PORT, PIN_CHL2);
    state->flt1 = HAL_GPIO_ReadPin(INPUT_PORT, PIN_FLT1);
    state->flt2 = HAL_GPIO_ReadPin(INPUT_PORT, PIN_FLT2);
    state->vdl1 = HAL_GPIO_ReadPin(INPUT_PORT, PIN_VDL1);
    state->vdl2 = HAL_GPIO_ReadPin(INPUT_PORT, PIN_VDL2);
}

int main(){
    HAL_Init();

    // Initialize LED
    Heartbeat_Clock_Init();
    GPIO_InitTypeDef led_config = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Pin = LED_PIN
    };
    HAL_GPIO_Init(LED_PORT, &led_config);

    // Initialize Your Inputs
    Init_System_Inputs();

    while(1){
        // Read all sensors into our struct
        Read_All_Inputs(&current_state);

        // LOGIC TEST EXAMPLE: 
        // If FLT1 is High (assuming active high fault), blink fast. Else blink slow.
        if (current_state.flt1 == GPIO_PIN_SET) {
             HAL_Delay(100); // Fast blink (Panic/Fault mode)
        } else {
             HAL_Delay(500); // Normal heartbeat
        }
        
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    }
}