// #include "stm32f0xx_hal.h" // In a real project, you would include the master HAL header.
#include "display.h"
#include "Application/Keypad/keypad.h" // Moved to Application layer
#include "uart.h"
#include "Application/IR/ir_tx.h" // Include the new IR header
#include <stdio.h> // For sprintf

// --- Menu System ---
const char* menu_items[] = {
    "Send NEC Command",
    "Learn New Command",
    "Saved Remotes",
    "Settings"
};
const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);
int current_selection = 0;
// --- End Menu System ---

// Placeholder for HAL_Delay
void HAL_Delay(volatile uint32_t ms) {
    volatile uint32_t i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 1000; j++);
    }
}

// Placeholder for SystemClock_Config
void SystemClock_Config(void) {}

// Placeholder for HAL_Init
void HAL_Init(void) {}

// --- UI Function ---
void draw_menu(void) {
    char buffer[64];
    UART_Transmit_String("\n--- IR Remote Menu ---\n");
    for (int i = 0; i < menu_size; i++) {
        if (i == current_selection) {
            sprintf(buffer, "> %s\n", menu_items[i]);
        } else {
            sprintf(buffer, "  %s\n", menu_items[i]);
        }
        UART_Transmit_String(buffer);
    }
    UART_Transmit_String("----------------------\n");
}
// --- End UI Function ---

void handle_key_press(KeyCode key) {
    int needs_redraw = 1; // Redraw by default

    switch (key) {
        case KEY_UP:
            current_selection--;
            if (current_selection < 0) {
                current_selection = menu_size - 1; // Wrap around
            }
            break;
        case KEY_DOWN:
            current_selection++;
            if (current_selection >= menu_size) {
                current_selection = 0; // Wrap around
            }
            break;
        case KEY_OK:
            // Check which item is selected and act accordingly
            if (current_selection == 0) { // "Send NEC Command"
                // Call the IR function with dummy values
                IR_Send_NEC(0x00, 0x01);
            } else {
                // For other items, just print a message for now
                char buffer[64];
                sprintf(buffer, "\n*** Action for '%s' not implemented yet. ***\n", menu_items[current_selection]);
                UART_Transmit_String(buffer);
            }
            // We don't want to redraw the menu after this, the IR function provides its own output
            needs_redraw = 0;
            break;
        case KEY_BACK:
            UART_Transmit_String("\n*** Back pressed ***\n");
            break;
        case KEY_LEFT:
        case KEY_RIGHT:
            // Do nothing for left/right in this menu
            needs_redraw = 0;
            break;
        default:
            needs_redraw = 0;
            break;
    }

    if (needs_redraw) {
        draw_menu();
    }
}

int main(void) {
    // 1. Initialize HAL and System Clock
    HAL_Init();
    SystemClock_Config();

    // 2. Initialize all configured peripherals
    Display_Init();
    Keypad_Init();
    UART_Init();

    UART_Transmit_String("System Initialized.\n");
    draw_menu(); // Initial menu draw

    // 3. Enter the main application loop
    while (1) {
        KeyCode key = Keypad_Scan();

        if (key != KEY_NONE) {
            handle_key_press(key);
        }

        HAL_Delay(10);
    }

    return 0;
}
