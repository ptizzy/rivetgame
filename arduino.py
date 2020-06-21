"""

Tools for interfacing with Arduinos using Python

"""
from __future__ import print_function
from builtins import input
import os
import platform
import time
import datetime
from collections import deque
import serial
import glob
import struct


class ArduinoInterface:
    """Dedicated controller for a single Arduino"""

    def __init__(self, port='COM6', baudrate=9600):
        """Set up the specs for a serial connection"""
        self.port = port
        self.baudrate = baudrate
        self.hue = 127
        self.animation = 2
        self.interface = None
        self.msg_dict = {}

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def open(self):
        """Open a serial connection to an arduino-compatible device"""
        try:
            if self.interface is None or not isinstance(self.interface, serial.Serial):
                self.interface = serial.Serial(self.port, self.baudrate)
            print("initialising")
            # Open the Serial Connection
            if not self.interface.isOpen():
                self.interface.open()
            print("arduino connection open")
            return True
        except Exception as arduino_err:
            print('arduino connection failed')
            print(arduino_err)
            return False

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
        line = self.interface.readline()
        if line is not None:
            new_msg = chr(line[0])
            try:
                msg_val = int(line[1:-2])
            except ValueError:
                return {}
            print(new_msg, msg_val)
            if target_msg is not None and new_msg != target_msg:
                return {}
            # Return the message, value pair. Update the dict.
            self.msg_dict.update({new_msg: msg_val})
            return {new_msg: msg_val}

    def read_serial_blocking(self, target_msg):
        """Wait until the current message is received."""
        current_msg = {}
        while current_msg == {}:
            current_msg = self.read_serial(target_msg)
        return current_msg


def list_serial_ports():
    """
    A function that tries to list serial ports on most common platforms
    Src http://stackoverflow.com/questions/11303850/
                what-is-the-cross-platform-method-of-enumerating-serial-ports-in-python-includi
    """
    system_name = platform.system()
    if system_name == "Windows":
        # Scan for available ports.
        available = []
        for i in range(256):
            try:
                s = serial.Serial(i)
                available.append(i)
                s.close()
            except serial.SerialException:
                pass
        return available
    elif system_name == "Darwin":
        # Mac
        return glob.glob('/dev/tty*') + glob.glob('/dev/cu*')
    else:
        # Assume Linux or something else
        return glob.glob('/dev/ttyS*') + glob.glob('/dev/ttyUSB*')


if __name__ == '__main__':

    # List the serial devices available
    list_serial_ports()

    # Set up an Arduino
    str_port = 'COM3'
    # str_port = '/dev/tty.usbserial-A7006Yqh'  # <--- How to do this in Linux (see list_serial_ports)
    arduino = ArduinoInterface(port=str_port, baudrate=115200)  # Set up an Arduino

    print(arduino.read_serial())
