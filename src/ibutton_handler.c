#include "ibutton_handler.h"
#include "command.h"
#include "uart.h"
#include "ibutton_driver.h"
#include <string.h>

#define CMD_IBUTTON_READ_ID 0x30

/**
 * @brief Handler for the "Read iButton ID" command.
 */
static void handle_ibutton_read_id(const uint8_t *data, uint16_t len) {
    uint64_t id = 0;

    // Call the driver to read the ID
    if (iButton_Driver_Read_ID(&id)) {
        // Success, send the 8-byte ID back as the payload
        UART_SendFrame(CMD_IBUTTON_READ_ID, (const uint8_t*)&id, sizeof(id));
    } else {
        // Failure, send an error response
        const char* response = "ERROR: Failed to read iButton ID.";
        UART_SendFrame(0xFF, (const uint8_t*)response, strlen(response));
    }
}

/**
 * @brief Initializes the iButton module.
 */
void iButton_Init(void) {
    iButton_Driver_Init();
    Command_Register(CMD_IBUTTON_READ_ID, handle_ibutton_read_id);
}
