#ifndef IR_TX_H
#define IR_TX_H

#include <stdint.h> // For uint32_t

/**
 * @brief Simulates sending an IR command using the NEC protocol.
 *
 * In a real application, this would control a timer and GPIO to generate
 * a 38kHz modulated IR signal. Here, it just prints the steps to the UART.
 *
 * @param address The 16-bit address of the device.
 * @param command The 16-bit command.
 */
void IR_Send_NEC(uint16_t address, uint16_t command);

#endif // IR_TX_H
