#include "stm32xx_hal.h"

// --- 1. PIN DEFINITIONS ---

// Port A
#define PIN_SDCARD      GPIO_PIN_9      // Input

// Port B
#define PIN_DISABLE1    GPIO_PIN_4      // Input
#define PIN_SR_DATA     GPIO_PIN_5      // OUTPUT
#define PIN_BOOT_SW1    GPIO_PIN_7      // Input
#define PIN_BOOT_SW2    GPIO_PIN_8      // Input
// (Plus your previous Port B pins...)
#define PIN_VDL2        GPIO_PIN_9      // Input
#define PIN_CHL2        GPIO_PIN_12     // Input
#define PIN_FLT1        GPIO_PIN_13     // Input
#define PIN_FLT2        GPIO_PIN_14     // Input
#define PIN_VDL1        GPIO_PIN_15     // Input

// Port C
#define PIN_SHDN        GPIO_PIN_10     // Input
#define PIN_DISABLE     GPIO_PIN_11     // Input

// Port G
#define PIN_RESET_BTN   GPIO_PIN_10     // Input

// --- 2. DATA STRUCTURE (Inputs Only) ---
// We use bit-fields (uint8_t) to save space, but standard bool/uint8_t is fine too.
typedef struct {
    // Port A
    uint8_t sdcard_detect;
    
    // Port B
    uint8_t disable1;
    uint8_t boot_sw1;
    uint8_t boot_sw2;
    uint8_t vdl2;
    uint8_t chl2;
    uint8_t flt1;
    uint8_t flt2;
    uint8_t vdl1;
    
    // Port C
    uint8_t shdn;
    uint8_t disable_main;
    
    // Port G
    uint8_t reset_btn;
} InputState_t;

InputState_t current_state;

// --- 3. SYSTEM INITIALIZATION ---
void Init_System_GPIO(void) {
    
    // 1. Enable Clocks for ALL required ports
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE(); // Make sure your specific MCU has Port G!

    GPIO_InitTypeDef init_config = {0};

    // --- GROUP 1: INPUTS (Port A) ---
    init_config.Pin = PIN_SDCARD;
    init_config.Mode = GPIO_MODE_INPUT;
    init_config.Pull = GPIO_NOPULL; // Change to PULLUP if needed
    HAL_GPIO_Init(GPIOA, &init_config);

    // --- GROUP 2: INPUTS (Port B) ---
    // We bitwise OR all input pins for Port B
    init_config.Pin = PIN_DISABLE1 | PIN_BOOT_SW1 | PIN_BOOT_SW2 | 
                      PIN_VDL2 | PIN_CHL2 | PIN_FLT1 | PIN_FLT2 | PIN_VDL1;
    init_config.Mode = GPIO_MODE_INPUT;
    init_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &init_config);

    // --- GROUP 3: INPUTS (Port C) ---
    init_config.Pin = PIN_SHDN | PIN_DISABLE;
    init_config.Mode = GPIO_MODE_INPUT;
    init_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &init_config);

    // --- GROUP 4: INPUTS (Port G) ---
    init_config.Pin = PIN_RESET_BTN;
    init_config.Mode = GPIO_MODE_INPUT;
    init_config.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOG, &init_config);

    // --- GROUP 5: OUTPUTS (Port B) ---
    // SR_Data is an Output, so it needs a separate config
    init_config.Pin = PIN_SR_DATA;
    init_config.Mode = GPIO_MODE_OUTPUT_PP; // Push-Pull Output
    init_config.Pull = GPIO_NOPULL;
    init_config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &init_config);
    
    // Optional: Set default state for Output
    HAL_GPIO_WritePin(GPIOB, PIN_SR_DATA, GPIO_PIN_RESET);
}

// --- 4. READ FUNCTION ---
void Read_All_Inputs(InputState_t *state) {
    // Port A
    state->sdcard_detect = HAL_GPIO_ReadPin(GPIOA, PIN_SDCARD);

    // Port B
    state->disable1 = HAL_GPIO_ReadPin(GPIOB, PIN_DISABLE1);
    state->boot_sw1 = HAL_GPIO_ReadPin(GPIOB, PIN_BOOT_SW1);
    state->boot_sw2 = HAL_GPIO_ReadPin(GPIOB, PIN_BOOT_SW2);
    state->vdl2     = HAL_GPIO_ReadPin(GPIOB, PIN_VDL2);
    state->chl2     = HAL_GPIO_ReadPin(GPIOB, PIN_CHL2);
    state->flt1     = HAL_GPIO_ReadPin(GPIOB, PIN_FLT1);
    state->flt2     = HAL_GPIO_ReadPin(GPIOB, PIN_FLT2);
    state->vdl1     = HAL_GPIO_ReadPin(GPIOB, PIN_VDL1);

    // Port C
    state->shdn         = HAL_GPIO_ReadPin(GPIOC, PIN_SHDN);
    state->disable_main = HAL_GPIO_ReadPin(GPIOC, PIN_DISABLE);

    // Port G
    state->reset_btn = HAL_GPIO_ReadPin(GPIOG, PIN_RESET_BTN);
}

// --- MAIN LOOP EXAMPLE ---
int main(void) {
    HAL_Init();
    Init_System_GPIO();

    while (1) {
        Read_All_Inputs(&current_state);

        // Example: If Reset Button is pressed (assuming Active High), toggle SR_Data output
        if (current_state.reset_btn == GPIO_PIN_SET) {
            HAL_GPIO_TogglePin(GPIOB, PIN_SR_DATA);
            HAL_Delay(200); // Debounce delay
        }

        HAL_Delay(10);
    }
}