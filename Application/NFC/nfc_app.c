#include "nfc_app.h"
#include "uart.h"   // For UART_Transmit_String
#include <stdio.h>  // For sprintf

// --- NFC Application State ---
static NfcAppState current_nfc_state = NFC_APP_STATE_IDLE;

// --- NFC Sub-Menu ---
const char* nfc_menu_items[] = {
    "Read Card",
    "Emulate Card",
    "Saved Cards",
    "Exit"
};
const int nfc_menu_size = sizeof(nfc_menu_items) / sizeof(nfc_menu_items[0]);
static int nfc_current_selection = 0;


static void nfc_draw_menu(void) {
    char buffer[64];
    UART_Transmit_String("\n--- NFC Menu ---\n");
    for (int i = 0; i < nfc_menu_size; i++) {
        if (i == nfc_current_selection) {
            sprintf(buffer, "> %s\n", nfc_menu_items[i]);
        } else {
            sprintf(buffer, "  %s\n", nfc_menu_items[i]);
        }
        UART_Transmit_String(buffer);
    }
    UART_Transmit_String("----------------\n");
}


void NFC_App_Init(void) {
    UART_Transmit_String("\n[NFC App] Initializing...\n");
    current_nfc_state = NFC_APP_STATE_MENU;
    nfc_current_selection = 0;
    nfc_draw_menu();
}

void NFC_App_Loop(void) {
    // For now, the loop doesn't do anything on its own.
    // In a real app, this is where we would poll for a card if in reading mode.
}

// Forward declaration for the delay function from main.c
extern void HAL_Delay(volatile uint32_t ms);

// This function is a public getter, which we will need in main.c
NfcAppState NFC_App_Get_State(void) {
    return current_nfc_state;
}

void NFC_App_Handle_Key(KeyCode key) {
    char buffer[64];
    int needs_redraw = 0;

    switch (current_nfc_state) {
        case NFC_APP_STATE_MENU:
            switch (key) {
                case KEY_UP:
                    nfc_current_selection--;
                    if (nfc_current_selection < 0) nfc_current_selection = nfc_menu_size - 1;
                    needs_redraw = 1;
                    break;
                case KEY_DOWN:
                    nfc_current_selection++;
                    if (nfc_current_selection >= nfc_menu_size) nfc_current_selection = 0;
                    needs_redraw = 1;
                    break;
                case KEY_OK:
                    // Handle selection
                    switch (nfc_current_selection) {
                        case 0: // Read Card
                            current_nfc_state = NFC_APP_STATE_READING;
                            UART_Transmit_String("\n[NFC Read] Polling for card...\n");
                            HAL_Delay(2000); // Simulate waiting for a card
                            UART_Transmit_String("[NFC Read] Card Found! Type: Mifare Classic, UID: 0xDEADBEEF\n");
                            HAL_Delay(1000);
                            current_nfc_state = NFC_APP_STATE_MENU;
                            needs_redraw = 1;
                            break;
                        case 1: // Emulate Card
                            current_nfc_state = NFC_APP_STATE_EMULATING;
                            UART_Transmit_String("\n[NFC Emulate] Emulating card UID: 0xCAFEBABE\n");
                            UART_Transmit_String("[NFC Emulate] Press BACK to stop.\n");
                            break;
                        case 3: // Exit
                            current_nfc_state = NFC_APP_STATE_IDLE;
                            UART_Transmit_String("\n[NFC App] Exiting to main menu.\n");
                            break;
                        default:
                            sprintf(buffer, "\n[NFC App] '%s' not implemented.\n", nfc_menu_items[nfc_current_selection]);
                            UART_Transmit_String(buffer);
                            break;
                    }
                    break;
                case KEY_BACK:
                    current_nfc_state = NFC_APP_STATE_IDLE;
                    UART_Transmit_String("\n[NFC App] Exiting to main menu.\n");
                    break;
                default:
                    break;
            }
            break;

        case NFC_APP_STATE_EMULATING:
            if (key == KEY_BACK) {
                current_nfc_state = NFC_APP_STATE_MENU;
                UART_Transmit_String("\n[NFC Emulate] Emulation stopped.\n");
                needs_redraw = 1;
            }
            break;

        case NFC_APP_STATE_READING:
            // Input is ignored while "reading"
            break;

        case NFC_APP_STATE_IDLE:
            // Should not handle keys in idle state
            break;
    }

    if (needs_redraw) {
        nfc_draw_menu();
    }
}
