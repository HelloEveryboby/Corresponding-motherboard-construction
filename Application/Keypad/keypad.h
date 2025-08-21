#ifndef KEYPAD_H
#define KEYPAD_H

#include "stm32f0xx_hal.h" // Placeholder for HAL types

// Key definitions based on the Flipper Zero style input
typedef enum {
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_OK,     // The 'Confirm' button in the middle of the D-pad
    KEY_BACK
} KeyCode;

// GPIO Pin definitions for the keypad, based on "超小型STM32红外遥控器 GPIO 分配"
// 5-way navigation key
#define KEY_UP_PORT         GPIOA
#define KEY_UP_PIN          GPIO_PIN_1
#define KEY_DOWN_PORT       GPIOA
#define KEY_DOWN_PIN        GPIO_PIN_2
#define KEY_LEFT_PORT       GPIOA
#define KEY_LEFT_PIN        GPIO_PIN_3
#define KEY_RIGHT_PORT      GPIOA
#define KEY_RIGHT_PIN       GPIO_PIN_4
#define KEY_OK_PORT         GPIOA
#define KEY_OK_PIN          GPIO_PIN_5

// Back button
#define KEY_BACK_PORT       GPIOA
#define KEY_BACK_PIN        GPIO_PIN_0


/**
 * @brief Initializes the GPIO pins for the keypad.
 *
 * Configures PA0 through PA5 as inputs with internal pull-up resistors.
 */
void Keypad_Init(void);

/**
 * @brief Scans the keypad for a single, debounced key press.
 *
 * This function should be called periodically in the main loop.
 * It includes a simple software debounce mechanism.
 *
 * @return The `KeyCode` of the pressed key, or `KEY_NONE` if no key is pressed.
 */
KeyCode Keypad_Scan(void);

#endif // KEYPAD_H
