#include "ring_buffer.h"
#include <string.h>

void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, size_t size) {
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

size_t RingBuffer_Write(RingBuffer_t *rb, const uint8_t *data, size_t len) {
    if (len > RingBuffer_GetFreeSpace(rb)) {
        // Not enough space, truncate the write
        len = RingBuffer_GetFreeSpace(rb);
    }

    for (size_t i = 0; i < len; i++) {
        rb->buffer[rb->head] = data[i];
        rb->head = (rb->head + 1) % rb->size;
    }
    rb->count += len;
    return len;
}

size_t RingBuffer_Read(RingBuffer_t *rb, uint8_t *data, size_t len) {
    if (len > rb->count) {
        // Not enough data, truncate the read
        len = rb->count;
    }

    for (size_t i = 0; i < len; i++) {
        data[i] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
    }
    rb->count -= len;
    return len;
}

size_t RingBuffer_Peek(RingBuffer_t *rb, uint8_t *data, size_t len) {
    if (len > rb->count) {
        len = rb->count;
    }

    size_t current_tail = rb->tail;
    for (size_t i = 0; i < len; i++) {
        data[i] = rb->buffer[current_tail];
        current_tail = (current_tail + 1) % rb->size;
    }
    return len;
}

size_t RingBuffer_GetCount(const RingBuffer_t *rb) {
    return rb->count;
}

size_t RingBuffer_GetFreeSpace(const RingBuffer_t *rb) {
    return rb->size - rb->count;
}
