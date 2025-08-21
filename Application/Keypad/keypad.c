#include "keypad.h"

// NOTE: The HAL_... function calls in this file are placeholders.
// To compile this, you would need to have the official STMicroelectronics
// HAL library linked in your project, which would provide the actual
// implementations for these functions. The logic, however, is sound.

// Debounce time in milliseconds. A key press will not be registered
// more than once within this time period.
#define DEBOUNCE_TIME_MS 50

static uint32_t last_press_tick = 0;

/**
 * @brief Initializes the GPIO pins for the keypad.
 */
void Keypad_Init(void) {
    // This is a placeholder for the real HAL GPIO Init structure
    // In a real project, this would be `GPIO_InitTypeDef GPIO_InitStruct = {0};`
    // and HAL_GPIO_Init would be called.

    // 1. Enable GPIOA clock
    // In a real project, this would be something like __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. Configure pins PA0-PA5 as input with pull-up
    // GPIO_InitStruct.Pin = KEY_UP_PIN | KEY_DOWN_PIN | KEY_LEFT_PIN | KEY_RIGHT_PIN | KEY_OK_PIN | KEY_BACK_PIN;
    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_PULLUP;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief Scans the keypad for a single, debounced key press.
 */
KeyCode Keypad_Scan(void) {
    // This is a placeholder for HAL_GetTick()
    // uint32_t current_tick = HAL_GetTick();
    uint32_t current_tick = 0; // Placeholder value

    // Check if the debounce time has passed since the last registered press
    if (current_tick - last_press_tick < DEBOUNCE_TIME_MS) {
        return KEY_NONE;
    }

    // This is a placeholder for HAL_GPIO_ReadPin
    // We assume GPIO_PIN_RESET means the key is pressed (connected to ground)
    #define FAKE_HAL_GPIO_ReadPin(port, pin) 1 // 1 == GPIO_PIN_SET (not pressed)

    if (FAKE_HAL_GPIO_ReadPin(KEY_UP_PORT, KEY_UP_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_UP;
    }
    if (FAKE_HAL_GPIO_ReadPin(KEY_DOWN_PORT, KEY_DOWN_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_DOWN;
    }
    if (FAKE_HAL_GPIO_ReadPin(KEY_LEFT_PORT, KEY_LEFT_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_LEFT;
    }
    if (FAKE_HAL_GPIO_ReadPin(KEY_RIGHT_PORT, KEY_RIGHT_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_RIGHT;
    }
    if (FAKE_HAL_GPIO_ReadPin(KEY_OK_PORT, KEY_OK_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_OK;
    }
    if (FAKE_HAL_GPIO_ReadPin(KEY_BACK_PORT, KEY_BACK_PIN) == 0) {
        last_press_tick = current_tick;
        return KEY_BACK;
    }

    return KEY_NONE;
}
