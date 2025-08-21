from uart_manager import UARTManager
import time
import struct

# --- Configuration ---
SERIAL_PORT = '/dev/ttyS0' # Placeholder for the serial port
BAUDRATE = 115200

# --- Command Definitions ---
CMD_GET_STATUS = 0x01
CMD_SEND_IR = 0x10
CMD_READ_IBUTTON = 0x30

def test_get_status(uart: UARTManager):
    """Tests the GET_STATUS command."""
    print("\n--- Running Test: Get Status ---")
    print(f"Sending GET_STATUS command (ID: {CMD_GET_STATUS})...")

    if not uart.send_frame(CMD_GET_STATUS):
        print("FAIL: Failed to send command.")
        return

    print("Waiting for response...")
    cmd_id, payload = uart.receive_frame()

    if cmd_id == CMD_GET_STATUS and payload and "STATUS: System OK" in payload.decode():
        print(f"SUCCESS: Received expected status message: '{payload.decode()}'")
    else:
        print(f"FAIL: Received unexpected response. CMD_ID: {cmd_id}, Payload: {payload}")

def test_ir_send(uart: UARTManager):
    """Tests the SEND_IR command."""
    print("\n--- Running Test: Send IR ---")

    # Construct a sample payload: 2-byte address, 2-byte command
    ir_address = 0x00FF
    ir_command = 0x1234
    ir_payload = struct.pack('<HH', ir_address, ir_command) # Pack as little-endian

    print(f"Sending SEND_IR command (ID: {CMD_SEND_IR}) with Address={ir_address:04X}, Command={ir_command:04X}...")

    if not uart.send_frame(CMD_SEND_IR, ir_payload):
        print("FAIL: Failed to send command.")
        return

    print("Waiting for response...")
    cmd_id, payload = uart.receive_frame()

    if cmd_id == CMD_SEND_IR and payload and "ACK: IR signal sent" in payload.decode():
        print(f"SUCCESS: Received expected ACK: '{payload.decode()}'")
    else:
        print(f"FAIL: Received unexpected response. CMD_ID: {cmd_id}, Payload: {payload}")

def test_ibutton_read(uart: UARTManager):
    """Tests the READ_IBUTTON command."""
    print("\n--- Running Test: Read iButton ---")
    print(f"Sending READ_IBUTTON command (ID: {CMD_READ_IBUTTON})...")

    if not uart.send_frame(CMD_READ_IBUTTON):
        print("FAIL: Failed to send command.")
        return

    print("Waiting for response...")
    # This might take longer if it's waiting for a real iButton, so use a longer timeout
    cmd_id, payload = uart.receive_frame(timeout_override=5.0)

    if cmd_id == CMD_READ_IBUTTON:
        if len(payload) == 8:
            # Unpack the 64-bit ID
            ibutton_id = struct.unpack('<Q', payload)[0]
            print(f"SUCCESS: Received iButton ID: {ibutton_id:016X}")
        else:
            print(f"FAIL: Expected 8-byte payload for iButton ID, but got {len(payload)} bytes.")
    elif payload and "ERROR" in payload.decode():
        print(f"INFO: Received expected error (no iButton present): '{payload.decode()}'")
    else:
        print(f"FAIL: Received unexpected response. CMD_ID: {cmd_id}, Payload: {payload}")


def main():
    """Main application logic."""
    print("--- Raspberry Pi Host Application ---")
    uart = UARTManager(SERIAL_PORT, BAUDRATE)

    try:
        if not uart.connect():
            print("Failed to connect to STM32. Exiting.")
            return

        print("Connection established.")
        time.sleep(1) # Give STM32 a moment to be ready

        # Run all tests
        test_get_status(uart)
        time.sleep(0.5)
        test_ir_send(uart)
        time.sleep(0.5)
        test_ibutton_read(uart)

    except KeyboardInterrupt:
        print("\nExiting on user request.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        print("\nDisconnecting...")
        uart.disconnect()

if __name__ == "__main__":
    main()
