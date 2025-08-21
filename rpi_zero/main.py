from uart_manager import UARTManager
import time

# --- Configuration ---
SERIAL_PORT = '/dev/ttyS0' # Placeholder for the serial port
BAUDRATE = 115200

# --- Command Definitions ---
CMD_GET_STATUS = 0x01

def main():
    """Main application logic."""
    print("--- Raspberry Pi Host Application ---")

    # Instantiate the UART manager
    uart = UARTManager(SERIAL_PORT, BAUDRATE)

    try:
        # Connect to the STM32
        if not uart.connect():
            print("Failed to connect to STM32. Exiting.")
            return

        print("Connection established.")

        # --- End-to-End Test ---
        print("\n--- Running End-to-End Test: Get Status ---")
        print(f"Sending GET_STATUS command (ID: {CMD_GET_STATUS})...")

        # Send the command
        success = uart.send_frame(CMD_GET_STATUS)

        if not success:
            print("Failed to send command.")
            return

        # Wait for the response
        print("Waiting for response...")
        response_cmd_id, payload = uart.receive_frame()

        # Validate the response
        if response_cmd_id is not None:
            print(f"Received response. CMD_ID: {response_cmd_id}")
            if response_cmd_id == CMD_GET_STATUS:
                try:
                    # Decode payload as a UTF-8 string
                    status_message = payload.decode('utf-8')
                    print(f"Payload: '{status_message}'")
                    if status_message == "STATUS: System OK":
                        print("SUCCESS: Received expected status message.")
                    else:
                        print("ERROR: Received unexpected status message.")
                except UnicodeDecodeError:
                    print(f"ERROR: Could not decode payload: {payload}")
            else:
                print(f"ERROR: Received response for wrong command ID. Expected {CMD_GET_STATUS}.")
        else:
            print("ERROR: Did not receive a valid response (timeout or CRC error).")

    except KeyboardInterrupt:
        print("\nExiting on user request.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    finally:
        # Ensure the connection is closed cleanly
        print("\nDisconnecting...")
        uart.disconnect()


if __name__ == "__main__":
    main()
