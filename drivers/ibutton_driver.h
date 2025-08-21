#ifndef IBUTTON_DRIVER_H
#define IBUTTON_DRIVER_H

#include <stdint.h>

/**
 * @brief Initializes the iButton driver.
 *
 * Configures the GPIO pin for 1-Wire communication.
 */
void iButton_Driver_Init(void);

/**
 * @brief Reads the 64-bit (8-byte) ROM ID from an iButton device.
 *
 * @param id A pointer to a uint64_t to store the read ID.
 * @return 1 on success (ID read and CRC valid), 0 on failure.
 */
uint8_t iButton_Driver_Read_ID(uint64_t *id);

// Note: Write and emulate functions from the markdown are more complex
// and will be omitted for this initial integration.

#endif // IBUTTON_DRIVER_H
