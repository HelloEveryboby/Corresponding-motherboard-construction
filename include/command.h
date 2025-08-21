#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>
#include <stddef.h>

// Defines the function signature for a command handler.
// Any function that processes a command must match this type.
typedef void (*command_handler_t)(const uint8_t *data, uint16_t len);

/**
 * @brief Initializes the command processing module.
 */
void Command_Init(void);

/**
 * @brief Registers a handler function for a specific command ID.
 *
 * @param cmd_id The ID of the command (e.g., 0x01 for "Get Status").
 * @param handler The function pointer to the handler for this command.
 * @return 0 on success, -1 on failure (e.g., registry is full).
 */
int Command_Register(uint8_t cmd_id, command_handler_t handler);

/**
 * @brief This is the main entry point for processing frames from the UART driver.
 *
 * This function is designed to be passed to `UART_Init` as the frame callback.
 * It looks up the command ID in the registry and calls the appropriate handler.
 *
 * @param cmd_id The command ID from the received frame.
 * @param data Pointer to the data payload of the frame.
 * @param len The length of the data payload.
 */
void Command_Process_Frame(uint8_t cmd_id, const uint8_t *data, uint16_t len);


#endif // COMMAND_H
