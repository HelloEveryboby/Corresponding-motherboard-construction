#ifndef IR_DRIVER_H
#define IR_DRIVER_H

#include <stdint.h>

/**
 * @brief Initializes the IR driver.
 *
 * This function should configure the GPIO pin for the IR LED and the
 * timer peripheral used to generate the 38kHz carrier wave.
 */
void IR_Driver_Init(void);

/**
 * @brief Sends an IR command using the NEC protocol.
 *
 * This function generates the specific pulse train for the NEC protocol
 * to transmit an address and a command.
 *
 * @param address The 16-bit address (often 8-bit device, 8-bit inverse).
 * @param command The 16-bit command (often 8-bit command, 8-bit inverse).
 */
void IR_Driver_Send_NEC(uint16_t address, uint16_t command);


#endif // IR_DRIVER_H
