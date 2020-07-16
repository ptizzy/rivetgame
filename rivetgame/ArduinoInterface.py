import struct
import time

import pygame
import serial


class BaseArduinoInterface:

    def __init__(self, msg_dict):
        self.msg_dict = msg_dict
        pygame.mixer.init()
        self.player1_sound_correct = pygame.mixer.Sound("sounds/rivet1.wav")
        self.player2_sound_correct = pygame.mixer.Sound("sounds/rivet2.wav")
        self.player_sound_wrong = pygame.mixer.Sound("sounds/error.wav")
        self.start_time = time.time()

    def get_state(self):
        return self.msg_dict.get("S", 0)

    def get_rivets(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("R", 0)
        elif player_num == 1:
            return self.msg_dict.get("r", 0)

    def get_points(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("P", 0)
        elif player_num == 1:
            return self.msg_dict.get("p", 0)

    def get_combo(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("C", 0)
        elif player_num == 1:
            return self.msg_dict.get("c", 0)

    def get_rivet_attempts(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("M", 0)
        elif player_num == 1:
            return self.msg_dict.get("m", 0)

    def get_angle(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("A", 0)
        elif player_num == 1:
            return self.msg_dict.get("a", 0)

    def get_remaining_time(self):
        return self.msg_dict.get("T", 0)

    def get_start_time(self):
        return self.start_time

    def read_serial(self, target_msg=None):
        pass


class ArduinoInterface(BaseArduinoInterface):
    """Dedicated controller for a single Arduino"""

    def __init__(self, port='COM6', baudrate=9600):
        """Set up the specs for a serial connection"""
        super().__init__({})
        self.port = port
        self.baudrate = baudrate
        self.hue = 127
        self.animation = 2
        self.interface = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def open(self):
        """Open a serial connection to an arduino-compatible device"""
        # try:
        if self.interface is None or not isinstance(self.interface, serial.Serial):
            self.interface = serial.Serial(self.port, self.baudrate, timeout=0)
        print("initialising")
        # Open the Serial Connection
        if not self.interface.isOpen():
            self.interface.open()
        print("arduino connection open")
        return True
        # except Exception as arduino_err:
        #     print('arduino connection failed')
        #     print(arduino_err)
        #     return False

    def close(self):
        """Close the connection to an arduino-compatible device"""
        self.interface.flush()
        self.interface.close()

    def write_serial(self, msg, value):
        """
        Write a serial message to the arduino.
        Adapt to all of the overkill requirements of Python 3 Serial
        """
        self.interface.write(msg.encode())
        self.interface.write(struct.pack('>B', int(value)))

    def read_serial(self, target_msg=None):
        """Read a single line from the Arduino"""
        # try:
        line = self.interface.readline().strip()
        if line is None or len(line) == 0:
            return

        new_msg = line[0]
        try:
            new_msg = chr(new_msg)
        except ValueError:
            return {}

        msg_val = line[1:]
        try:
            if "." in str(msg_val):
                msg_val = float(msg_val)
            else:
                msg_val = int(msg_val)
        except ValueError:
            return {}
        print(new_msg, msg_val)

        if new_msg == "E":
            raise Exception("Error on arduino restarting")
        if new_msg == "S":
            self.start_time = time.time()

        if target_msg is not None and new_msg != target_msg:
            return {}

        self.play_sound(new_msg, msg_val)

        # Return the message, value pair. Update the dict.
        self.msg_dict.update({new_msg: msg_val})
        return {new_msg: msg_val}
        # except Exception as e:
        #     print("Exception", e)
        #     return {}

    def read_serial_blocking(self, target_msg):
        """Wait until the current message is received."""
        current_msg = {}
        while current_msg == {}:
            current_msg = self.read_serial(target_msg)
        return current_msg

    def play_sound(self, command, value):
        # Successful
        if command != "V":
            return
        if value == 1:
            pygame.mixer.Sound.play(self.player1_sound_correct)
        elif value == 2:
            pygame.mixer.Sound.play(self.player2_sound_correct)
        elif value == 3:
            pygame.mixer.Sound.play(self.player_sound_wrong)