#include "nfc_handler.h"
#include "command.h"
#include "uart.h"
#include <string.h>

// Placeholder for NFC command handlers
// In a real application, these would interact with an NFC driver (e.g., for a PN532 or ST25R3916)

/**
 * @brief Handler for a "Get NFC Status" command.
 */
static void handle_nfc_get_status(const uint8_t *data, uint16_t len) {
    const char* response = "ACK: NFC Status: OK";
    UART_SendFrame(0x20, (const uint8_t*)response, strlen(response));
}

/**
 * @brief Handler for a "Start NFC Scan" command.
 */
static void handle_nfc_scan(const uint8_t *data, uint16_t len) {
    const char* response = "ACK: NFC Scan initiated...";
    UART_SendFrame(0x21, (const uint8_t*)response, strlen(response));
    // In a real implementation, this would start a non-blocking scan process.
    // The result of the scan would be sent back in a different, unsolicited frame.
}

/**
 * @brief Initializes the NFC module and registers its command handlers.
 */
void NFC_Init(void) {
    Command_Register(0x20, handle_nfc_get_status);
    Command_Register(0x21, handle_nfc_scan);
}
