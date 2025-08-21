#ifndef UART_H
#define UART_H

#include "stm32f0xx_hal.h" // Placeholder for HAL types

// UART Pin definitions based on "超小型STM32红外遥控器 GPIO 分配"
#define DEBUG_UART_PORT     GPIOA
#define DEBUG_UART_TX_PIN   GPIO_PIN_9
#define DEBUG_UART_RX_PIN   GPIO_PIN_10

#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

/**
 * @brief Callback function type for when a full, valid frame is received.
 * @param cmd The command ID from the frame.
 * @param data Pointer to the data payload.
 * @param len Length of the data payload.
 */
typedef void (*uart_frame_callback_t)(uint8_t cmd, const uint8_t *data, uint16_t len);

/**
 * @brief Initializes the UART peripheral and the driver.
 * @param frame_callback A function pointer that will be called when a valid frame is received.
 */
void UART_Init(uart_frame_callback_t frame_callback);

/**
 * @brief Sends a data frame over UART.
 *
 * This function will construct a frame with the specified command and data,
 * calculate the CRC, and send it over UART.
 *
 * @param cmd The command ID.
 * @param data Pointer to the data payload.
 * @param len Length of the data payload.
 */
void UART_SendFrame(uint8_t cmd, const uint8_t *data, uint16_t len);

/**
 * @brief This function should be called periodically from the main loop.
 *
 * It processes the data in the ring buffer and parses frames.
 */
void UART_Process(void);

/**
 * @brief Simulates a UART Receive Interrupt.
 *
 * In a real application, the HAL_UART_RxCpltCallback would call this function
 * or place the received byte into the ring buffer directly.
 * For our simulation, we can call this to simulate byte-by-byte reception.
 *
 * @param byte The byte received from UART.
 */
void UART_Receive_IT(uint8_t byte);


#endif // UART_H
