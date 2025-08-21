#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>

// A simple ring buffer structure for UART communication
typedef struct {
    uint8_t *buffer;
    size_t size;
    size_t head;
    size_t tail;
    size_t count;
} RingBuffer_t;

/**
 * @brief Initializes a ring buffer.
 * @param rb Pointer to the RingBuffer_t struct.
 * @param buffer Pointer to the underlying byte array.
 * @param size The size of the byte array.
 */
void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, size_t size);

/**
 * @brief Writes data to the ring buffer.
 * @param rb Pointer to the RingBuffer_t struct.
 * @param data Pointer to the data to be written.
 * @param len The number of bytes to write.
 * @return The number of bytes actually written.
 */
size_t RingBuffer_Write(RingBuffer_t *rb, const uint8_t *data, size_t len);

/**
 * @brief Reads data from the ring buffer.
 * @param rb Pointer to the RingBuffer_t struct.
 * @param data Pointer to a buffer to store the read data.
 * @param len The number of bytes to read.
 * @return The number of bytes actually read.
 */
size_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data, size_t len);

/**
 * @brief Peeks at data in the ring buffer without removing it.
 * @param rb Pointer to the RingBuffer_t struct.
 * @param data Pointer to a buffer to store the peeked data.
 * @param len The number of bytes to peek.
 * @return The number of bytes actually peeked.
 */
size_t RingBuffer_Peek(RingBuffer_t *rb, uint8_t *data, size_t len);

/**
 * @brief Gets the number of bytes currently stored in the buffer.
 * @param rb Pointer to the RingBuffer_t struct.
 * @return The number of bytes available to read.
 */
size_t RingBuffer_GetCount(const RingBuffer_t *rb);

/**
 * @brief Gets the amount of free space in the buffer.
 * @param rb Pointer to the RingBuffer_t struct.
 * @return The number of bytes that can be written.
 */
size_t RingBuffer_GetFreeSpace(const RingBuffer_t *rb);

#endif // RING_BUFFER_H
