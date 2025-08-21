#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculates the CRC-16-CCITT value for a block of data.
 *
 * This function implements the CRC-16 algorithm with the polynomial 0x1021.
 * It is used to validate the integrity of received frames and to generate
 * the CRC for transmitted frames.
 *
 * @param data Pointer to the data buffer.
 * @param len The length of the data in bytes.
 * @return The 16-bit CRC value.
 */
uint16_t CRC_Calculate(const uint8_t *data, uint16_t len);


#endif // CRC_H
