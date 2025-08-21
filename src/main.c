// #include "stm32f0xx_hal.h"
#include "uart.h"
#include "command.h"
#include "display.h"
#include "keypad.h"
#include "ir_handler.h"
#include "nfc_handler.h"
#include "ibutton_handler.h" // Include the new iButton handler module

// Placeholder for HAL_Delay
void HAL_Delay(volatile uint32_t ms) {
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1000; j++);
    }
}

// Placeholder for SystemClock_Config
void SystemClock_Config(void) {
    // Empty placeholder
}

// Placeholder for HAL_Init
void HAL_Init(void) {
    // Empty placeholder
}

// --- Example Command Handler for Get Status ---
void handle_get_status_command(const uint8_t *data, uint16_t len);


int main(void) {
    // 1. Initialize HAL and System Clock
    HAL_Init();
    SystemClock_Config();

    // 2. Initialize command processor
    Command_Init();

    // 3. Initialize peripherals
    UART_Init(Command_Process_Frame);
    Display_Init();
    Keypad_Init();

    // 4. Initialize application modules, which will register their own commands
    IR_Init();
    NFC_Init();
    iButton_Init(); // Initialize the iButton module

    // Register any other general system commands
    Command_Register(0x01, handle_get_status_command);


    // 5. Enter the main application loop
    while (1) {
        UART_Process();
    }

    return 0;
}


/**
 * @brief Placeholder handler for a "Get Status" command.
 */
void handle_get_status_command(const uint8_t *data, uint16_t len) {
    const char* status = "STATUS: System OK";
    UART_SendFrame(0x01, (const uint8_t*)status, strlen(status));
}
