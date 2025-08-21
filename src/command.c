#include "command.h"
#include "uart.h" // For sending responses, e.g., "Unknown Command"

#define MAX_COMMANDS 16 // Maximum number of commands that can be registered

// Structure to hold a command and its handler
typedef struct {
    uint8_t cmd_id;
    command_handler_t handler;
} Command_t;

// The command registry
static Command_t command_registry[MAX_COMMANDS];
static uint8_t registered_commands_count = 0;

void Command_Init(void) {
    for (int i = 0; i < MAX_COMMANDS; i++) {
        command_registry[i].cmd_id = 0;
        command_registry[i].handler = NULL;
    }
    registered_commands_count = 0;
}

int Command_Register(uint8_t cmd_id, command_handler_t handler) {
    if (registered_commands_count >= MAX_COMMANDS) {
        // Registry is full
        return -1;
    }

    command_registry[registered_commands_count].cmd_id = cmd_id;
    command_registry[registered_commands_count].handler = handler;
    registered_commands_count++;

    return 0;
}

void Command_Process_Frame(uint8_t cmd_id, const uint8_t *data, uint16_t len) {
    // Look for the command in the registry
    for (int i = 0; i < registered_commands_count; i++) {
        if (command_registry[i].cmd_id == cmd_id) {
            // Found the handler, execute it
            command_registry[i].handler(data, len);
            return;
        }
    }

    // If we get here, no handler was found for the command ID
    // We can send an "Unknown Command" response back to the host
    uint8_t error_payload[] = { cmd_id };
    // Using a placeholder command ID for "error"
    UART_SendFrame(0xFF, error_payload, sizeof(error_payload));
}
