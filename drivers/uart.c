#include "uart.h"
#include "ring_buffer.h"
#include <string.h>

// Forward declaration for a CRC function we will create in the next step.
uint16_t CRC_Calculate(const uint8_t *data, uint16_t len);

// Driver state
typedef enum {
    STATE_WAITING_FOR_HEADER,
    STATE_WAITING_FOR_LENGTH,
    STATE_WAITING_FOR_DATA,
} ParserState_t;

// Static variables for the driver
static RingBuffer_t rx_ring_buffer;
static uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
static uart_frame_callback_t frame_callback = NULL;

static ParserState_t parser_state = STATE_WAITING_FOR_HEADER;
static uint8_t frame_buffer[UART_RX_BUFFER_SIZE];
static uint16_t frame_len = 0;
static uint16_t frame_bytes_received = 0;

// --- Public Functions ---

void UART_Init(uart_frame_callback_t callback) {
    RingBuffer_Init(&rx_ring_buffer, rx_buffer, UART_RX_BUFFER_SIZE);
    frame_callback = callback;
    parser_state = STATE_WAITING_FOR_HEADER;
    // In a real project, this is where HAL_UART_Init and HAL_UART_Receive_IT would be called.
}

void UART_Receive_IT(uint8_t byte) {
    RingBuffer_Write(&rx_ring_buffer, &byte, 1);
}

void UART_Process(void) {
    uint8_t byte;
    while (RingBuffer_Read(&rx_ring_buffer, &byte, 1)) {
        switch (parser_state) {
            case STATE_WAITING_FOR_HEADER:
                if (byte == 0xAA) {
                    frame_bytes_received = 0;
                    frame_buffer[frame_bytes_received++] = byte;
                    parser_state = STATE_WAITING_FOR_LENGTH;
                }
                break;

            case STATE_WAITING_FOR_LENGTH:
                frame_buffer[frame_bytes_received++] = byte;
                frame_len = byte; // The length byte itself
                if (frame_len < 4 || frame_len > UART_RX_BUFFER_SIZE - 2) {
                    // Invalid length, reset
                    parser_state = STATE_WAITING_FOR_HEADER;
                } else {
                    parser_state = STATE_WAITING_FOR_DATA;
                }
                break;

            case STATE_WAITING_FOR_DATA:
                frame_buffer[frame_bytes_received++] = byte;
                // Check if we have received the full frame (Header + Length + Payload + 2-byte CRC)
                if (frame_bytes_received >= frame_len + 2) {
                    // Full frame received, now validate CRC
                    uint16_t received_crc = (uint16_t)(frame_buffer[frame_bytes_received - 1] << 8) | frame_buffer[frame_bytes_received - 2];
                    uint16_t calculated_crc = CRC_Calculate(frame_buffer, frame_bytes_received - 2);

                    if (received_crc == calculated_crc) {
                        if (frame_callback) {
                            // Frame is valid, call the callback
                            uint8_t cmd = frame_buffer[2];
                            const uint8_t* data = &frame_buffer[3];
                            uint16_t data_len = frame_len - 4; // Length - Seq - Cmd - CRC(2)
                            frame_callback(cmd, data, data_len);
                        }
                    }
                    // Reset for the next frame regardless of CRC validity
                    parser_state = STATE_WAITING_FOR_HEADER;
                }
                break;
        }
    }
}

void UART_SendFrame(uint8_t cmd, const uint8_t *data, uint16_t len) {
    if (len > UART_TX_BUFFER_SIZE - 6) {
        // Data too long to fit in a frame
        return;
    }

    uint8_t frame[UART_TX_BUFFER_SIZE];
    uint16_t frame_idx = 0;

    frame[frame_idx++] = 0xAA; // Header
    frame[frame_idx++] = len + 4; // Frame Length (Seq + Cmd + Data + CRC)
    frame[frame_idx++] = 0; // Sequence number (placeholder)
    frame[frame_idx++] = cmd; // Command

    if (len > 0) {
        memcpy(&frame[frame_idx], data, len);
        frame_idx += len;
    }

    uint16_t crc = CRC_Calculate(frame, frame_idx);
    frame[frame_idx++] = crc & 0xFF;
    frame[frame_idx++] = (crc >> 8) & 0xFF;

    // In a real project, this would write to a TX ring buffer and start DMA/IT transfer
    // For now, this is a placeholder for the transmit logic.
    // HAL_UART_Transmit(&huart1, frame, frame_idx, HAL_MAX_DELAY);
}
