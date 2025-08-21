#ifndef NFC_APP_H
#define NFC_APP_H

#include "Application/Keypad/keypad.h" // For KeyCode

/**
 * @brief Defines the different states or modes for the NFC application.
 */
typedef enum {
    NFC_APP_STATE_IDLE,
    NFC_APP_STATE_MENU,
    NFC_APP_STATE_READING,
    NFC_APP_STATE_EMULATING,
} NfcAppState;

/**
 * @brief Initializes the NFC application.
 *
 * This function is called once when switching to the NFC application.
 * It sets up the initial state and draws the initial UI.
 */
void NFC_App_Init(void);

/**
 * @brief The main loop for the NFC application.
 *
 * This function should be called repeatedly when the system is in the NFC application state.
 * It handles the logic for the current NFC mode (e.g., polling for cards, updating UI).
 */
void NFC_App_Loop(void);

/**
 * @brief Handles key presses for the NFC application.
 *
 * @param key The `KeyCode` of the key that was pressed.
 */
void NFC_App_Handle_Key(KeyCode key);

/**
 * @brief Gets the current state of the NFC application.
 *
 * @return The current NfcAppState.
 */
NfcAppState NFC_App_Get_State(void);

#endif // NFC_APP_H
