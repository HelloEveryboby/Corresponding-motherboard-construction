import serial
import struct
import crcmod
import time

class UARTManager:
    """
    Manages the UART communication with the STM32 co-processor,
    handling frame construction, sending, receiving, and validation.
    """

    def __init__(self, port, baudrate=115200, timeout=1):
        """
        Initializes the UARTManager.
        :param port: The serial port to connect to (e.g., '/dev/ttyS0' or 'COM3').
        :param baudrate: The communication speed.
        :param timeout: The read timeout in seconds.
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.serial_conn = None

        # Set up the CRC-16-CCITT calculation function
        self.crc16_func = crcmod.mkCrcFun(0x11021, rev=False, initCrc=0xFFFF, xorOut=0x0000)

    def connect(self):
        """Establishes the serial connection."""
        if self.serial_conn and self.serial_conn.is_open:
            print("Already connected.")
            return True
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
            print(f"Successfully connected to {self.port}")
            return True
        except serial.SerialException as e:
            print(f"Error connecting to {self.port}: {e}")
            return False

    def disconnect(self):
        """Closes the serial connection."""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            print("Disconnected.")

    def send_frame(self, cmd_id, data=b''):
        """
        Constructs and sends a command frame to the STM32.
        :param cmd_id: The command ID byte.
        :param data: The data payload as a bytes object.
        """
        if not (self.serial_conn and self.serial_conn.is_open):
            print("Not connected.")
            return False

        try:
            # Frame Structure: [Header, Length, Seq, Cmd, Payload..., CRC_L, CRC_H]
            # Seq is a placeholder for now.
            seq = 0
            # len = Seq(1) + Cmd(1) + len(data) + CRC(2) -> this is wrong in the C code, should be len(payload)+4
            # The C code has it as len(payload)+4. So let's stick to that.
            length = len(data) + 4

            # Construct the frame header and payload
            frame_header = struct.pack('<BBBB', 0xAA, length, seq, cmd_id)
            frame_core = frame_header + data

            # Calculate CRC
            crc = self.crc16_func(frame_core)
            crc_bytes = struct.pack('<H', crc)

            # Final frame
            full_frame = frame_core + crc_bytes

            self.serial_conn.write(full_frame)
            return True
        except serial.SerialException as e:
            print(f"Error writing to serial port: {e}")
            return False

    def receive_frame(self, timeout_override=None):
        """
        Waits for and receives a response frame from the STM32.
        :param timeout_override: Optional timeout in seconds to override the default.
        :return: A tuple (cmd_id, payload) or (None, None) on timeout or error.
        """
        if not (self.serial_conn and self.serial_conn.is_open):
            print("Not connected.")
            return None, None

        start_time = time.time()
        read_timeout = timeout_override if timeout_override is not None else self.timeout

        state = 'WAITING_FOR_HEADER'
        frame_buffer = bytearray()
        expected_len = 0

        while time.time() - start_time < read_timeout:
            byte = self.serial_conn.read(1)
            if not byte:
                continue

            if state == 'WAITING_FOR_HEADER':
                if byte == b'\xaa':
                    frame_buffer.append(byte[0])
                    state = 'WAITING_FOR_LENGTH'

            elif state == 'WAITING_FOR_LENGTH':
                frame_buffer.append(byte[0])
                expected_len = byte[0]
                state = 'WAITING_FOR_DATA'

            elif state == 'WAITING_FOR_DATA':
                frame_buffer.append(byte[0])
                # Check if full frame is received (Header + Length + Payload + CRC)
                if len(frame_buffer) >= expected_len + 2:
                    # Full frame received, validate CRC
                    received_crc_bytes = frame_buffer[-2:]
                    received_crc = struct.unpack('<H', received_crc_bytes)[0]

                    data_to_check = frame_buffer[:-2]
                    calculated_crc = self.crc16_func(data_to_check)

                    if received_crc == calculated_crc:
                        # Frame is valid
                        cmd_id = frame_buffer[3]
                        payload = frame_buffer[4:-2]
                        return cmd_id, payload
                    else:
                        print(f"CRC mismatch! Got {received_crc}, expected {calculated_crc}")
                        return None, None

        # Timeout occurred
        return None, None
