#include "ir_handler.h"
#include "command.h"
#include "uart.h"
#include "ir_driver.h" // Include the new driver
#include <string.h>

// Define the command ID for sending an IR signal
#define CMD_ID_SEND_IR 0x10

/**
 * @brief Handler for the "Send IR" command.
 *
 * This function now parses the payload and uses the IR driver to send a signal.
 * Expected payload format: 4 bytes -> 2 bytes for address, 2 bytes for command (little-endian).
 *
 * @param data Pointer to the data payload from the command frame.
 * @param len The length of the data payload.
 */
static void handle_send_ir_command(const uint8_t *data, uint16_t len) {
    if (len < 4) {
        // Payload is too short
        const char* response = "ERROR: IR command requires 4-byte payload.";
        UART_SendFrame(0xFF, (const uint8_t*)response, strlen(response));
        return;
    }

    // Parse address and command from the payload (assuming little-endian)
    uint16_t address = (uint16_t)(data[1] << 8) | data[0];
    uint16_t command = (uint16_t)(data[3] << 8) | data[2];

    // Use the driver to send the signal
    IR_Driver_Send_NEC(address, command);

    // Acknowledge that the command was processed
    const char* response = "ACK: IR signal sent.";
    UART_SendFrame(CMD_ID_SEND_IR, (const uint8_t*)response, strlen(response));
}

/**
 * @brief Initializes the IR module.
 *
 * This function initializes the underlying driver and registers the
 * command handlers with the command processor.
 */
void IR_Init(void) {
    IR_Driver_Init();
    Command_Register(CMD_ID_SEND_IR, handle_send_ir_command);
}
