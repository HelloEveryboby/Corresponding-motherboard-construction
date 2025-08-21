// #include "stm32f0xx_hal.h" // In a real project, you would include the master HAL header.
#include "display.h"
#include "Application/Keypad/keypad.h" // Moved to Application layer
#include "uart.h"
#include "Application/IR/ir_tx.h" // Include the new IR header
#include "Application/NFC/nfc_app.h" // Include the new NFC header
#include <stdio.h> // For sprintf

// --- Main Application State ---
typedef enum {
    APP_STATE_MAIN_MENU,
    APP_STATE_IR_APP,
    APP_STATE_NFC_APP,
} AppState;

static AppState current_app_state = APP_STATE_MAIN_MENU;
// --- End Main Application State ---

// --- Main Menu System ---
const char* main_menu_items[] = {
    "Infrared Menu",
    "NFC Menu",
    "Settings"
};
const int main_menu_size = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
int main_current_selection = 0;
// --- End Main Menu System ---

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
void draw_main_menu(void) {
    char buffer[64];
    UART_Transmit_String("\n--- Main Menu ---\n");
    for (int i = 0; i < main_menu_size; i++) {
        if (i == main_current_selection) {
            sprintf(buffer, "> %s\n", main_menu_items[i]);
        } else {
            sprintf(buffer, "  %s\n", main_menu_items[i]);
        }
        UART_Transmit_String(buffer);
    }
    UART_Transmit_String("-----------------\n");
}
// --- End UI Function ---

void handle_main_menu_key_press(KeyCode key) {
    int needs_redraw = 1; // Redraw by default

    switch (key) {
        case KEY_UP:
            main_current_selection--;
            if (main_current_selection < 0) {
                main_current_selection = main_menu_size - 1;
            }
            break;
        case KEY_DOWN:
            main_current_selection++;
            if (main_current_selection >= main_menu_size) {
                main_current_selection = 0;
            }
            break;
        case KEY_OK:
            switch (main_current_selection) {
                case 0: // Infrared Menu
                    // This is where we would switch to an IR App state
                    // For now, we just call the function directly
                    IR_Send_NEC(0x00, 0x01);
                    break;
                case 1: // NFC Menu
                    current_app_state = APP_STATE_NFC_APP;
                    NFC_App_Init();
                    break;
                default:
                    {
                        char buffer[64];
                        sprintf(buffer, "\n*** Action for '%s' not implemented. ***\n", main_menu_items[main_current_selection]);
                        UART_Transmit_String(buffer);
                    }
                    break;
            }
            needs_redraw = 0;
            break;
        case KEY_BACK:
            // No action on back in main menu
            needs_redraw = 0;
            break;
        default:
            needs_redraw = 0;
            break;
    }

    if (needs_redraw) {
        draw_main_menu();
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();

    Display_Init();
    Keypad_Init();
    UART_Init();

    UART_Transmit_String("System Initialized.\n");
    draw_main_menu(); // Initial menu draw

    while (1) {
        // --- State Machine ---
        if (current_app_state == APP_STATE_NFC_APP) {
            if (NFC_App_Get_State() == NFC_APP_STATE_IDLE) {
                current_app_state = APP_STATE_MAIN_MENU;
                draw_main_menu();
            }
        }
        // --- End State Machine ---

        KeyCode key = Keypad_Scan();

        if (key != KEY_NONE) {
            switch (current_app_state) {
                case APP_STATE_MAIN_MENU:
                    handle_main_menu_key_press(key);
                    break;
                case APP_STATE_NFC_APP:
                    NFC_App_Handle_Key(key);
                    break;
                case APP_STATE_IR_APP:
                    // Placeholder for IR app key handler
                    break;
            }
        }

        HAL_Delay(10);
    }

    return 0;
}
