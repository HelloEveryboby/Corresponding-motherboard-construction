#include "crc.h"

/**
 * @brief Calculates the CRC-16-CCITT value for a block of data.
 */
uint16_t CRC_Calculate(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF; // Initial value for CRC-16-CCITT
    const uint16_t poly = 0x1021;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc = crc << 1;
            }
        }
    }

    return crc;
}
