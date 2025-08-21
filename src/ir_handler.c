#include "ir_handler.h"
#include "command.h"
#include "uart.h"
#include <string.h>

// Define the command ID for sending an IR signal
#define CMD_ID_SEND_IR 0x10

/**
 * @brief Handler for the "Send IR" command.
 * @param data Pointer to the data payload from the command frame.
 * @param len The length of the data payload.
 */
static void handle_send_ir_command(const uint8_t *data, uint16_t len) {
    // In a real application, this would parse the `data` buffer
    // to get IR protocol, address, and command, then use an IR driver to send it.

    // For our proof-of-concept, we just acknowledge the command.
    const char* response = "ACK: IR command processed.";

    // We send the response back using the same command ID for simplicity.
    UART_SendFrame(CMD_ID_SEND_IR, (const uint8_t*)response, strlen(response));
}

/**
 * @brief Initializes the IR module and registers its command handlers.
 */
void IR_Init(void) {
    Command_Register(CMD_ID_SEND_IR, handle_send_ir_command);
}
