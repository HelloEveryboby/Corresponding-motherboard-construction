#include "ir_tx.h"
#include "uart.h"   // For UART_Transmit_String
#include <stdio.h>  // For sprintf

// In a real implementation, this would be a precise microsecond delay.
// We use the millisecond delay as a stand-in for simulation.
extern void HAL_Delay(volatile uint32_t ms);

// Helper function to print data bits
static void ir_send_data_simulation(uint32_t data, int bits) {
    char buffer[64];
    for (int i = 0; i < bits; i++) {
        // Check the most significant bit of the remaining data
        if (data & (1UL << (bits - 1 - i))) {
            // Logic '1'
            sprintf(buffer, "  [LOGIC 1] Pulse: 560us, Space: 1690us\n");
        } else {
            // Logic '0'
            sprintf(buffer, "  [LOGIC 0] Pulse: 560us, Space:  560us\n");
        }
        UART_Transmit_String(buffer);
        HAL_Delay(1); // Simulate time passing
    }
}

void IR_Send_NEC(uint16_t address, uint16_t command) {
    char buffer[64];

    UART_Transmit_String("\n--- Simulating IR NEC Transmission ---\n");

    // Simulate enabling the 38kHz carrier
    UART_Transmit_String("[HW] Enabling 38kHz PWM carrier...\n");
    HAL_Delay(5);

    // 1. Send 9ms leading pulse burst
    UART_Transmit_String("[NEC] Sending 9ms leading pulse burst...\n");
    HAL_Delay(9);

    // 2. Send 4.5ms space
    UART_Transmit_String("[NEC] Sending 4.5ms space...\n");
    HAL_Delay(5);

    // 3. Send 8-bit address and its logical inverse
    UART_Transmit_String("[NEC] Sending 8-bit address and inverse...\n");
    uint8_t addr_l = address & 0xFF;
    uint8_t addr_h_inv = ~(address >> 8); // Assuming standard NEC, address is 8-bit. We'll send LSB and its inverse for this example.
    ir_send_data_simulation(addr_l, 8);
    ir_send_data_simulation(~addr_l, 8);

    // 4. Send 8-bit command and its logical inverse
    UART_Transmit_String("[NEC] Sending 8-bit command and inverse...\n");
    uint8_t cmd_l = command & 0xFF;
    uint8_t cmd_h_inv = ~(command >> 8);
    ir_send_data_simulation(cmd_l, 8);
    ir_send_data_simulation(~cmd_l, 8);

    // 5. Send final 560us pulse
    UART_Transmit_String("[NEC] Sending final 560us pulse...\n");
    HAL_Delay(1);

    // Simulate disabling the carrier
    UART_Transmit_String("[HW] Disabling PWM carrier.\n");

    UART_Transmit_String("--- IR Simulation Complete ---\n");
}
