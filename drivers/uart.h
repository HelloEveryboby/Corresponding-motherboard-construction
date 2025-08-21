#ifndef UART_H
#define UART_H

#include "stm32f0xx_hal.h" // Placeholder for HAL types

// UART Pin definitions based on "超小型STM32红外遥控器 GPIO 分配"
#define DEBUG_UART_PORT     GPIOA
#define DEBUG_UART_TX_PIN   GPIO_PIN_9
#define DEBUG_UART_RX_PIN   GPIO_PIN_10

/**
 * @brief Initializes the UART peripheral for debug output.
 *
 * Configures USART1 with the specified pins (PA9, PA10).
 * Baud rate is typically set to 115200.
 */
void UART_Init(void);

/**
 * @brief Transmits a null-terminated string over UART.
 *
 * @param str The string to transmit.
 */
void UART_Transmit_String(const char* str);


#endif // UART_H
