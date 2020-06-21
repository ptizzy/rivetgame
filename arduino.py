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


class AnalogData:
    """class that holds analog data for N samples"""

    def __init__(self, max_len):
        """Start with a deque object for each axis"""
        self.axis_x = deque([0.0] * max_len)
        self.axis_y1 = deque([0.0] * max_len)
        self.axis_y2 = deque([0.0] * max_len)
        self.max_len = max_len
        self.start = time.time()

    def add_to_buf(self, buf, val):
        """Use a ring buffer to manage the chart data"""
        if len(buf) < self.max_len:
            buf.append(val)
        else:
            buf.pop()
            buf.appendleft(val)

    def add(self, data):
        """Add time stamps to the data and add the data to the buffer"""
        assert len(data) == 2
        self.add_to_buf(self.axis_x, time.time() - self.start)
        self.add_to_buf(self.axis_y1, data[0])
        self.add_to_buf(self.axis_y2, data[1])


class ArduinoInterface:
    """Dedicated controller for a single Arduino"""

    def __init__(self, port='COM6', baudrate=9600):
        """Set up the specs for a serial connection"""
        self.port = port
        self.baudrate = baudrate
        self.hue = 127
        self.animation = 2
        self.interface = None
        assert self.open() is True
        self.analog_data = AnalogData(100)
        self.analog_plot = AnalogPlot()

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

    def read_serial(self, target_msg='d'):
        """Read a single line from the Arduino"""
        line = self.interface.readline()
        if line is not None:
            new_msg = chr(line[0])
            msg_val = int(line[1:-2])
            print(new_msg, msg_val)
            if new_msg == target_msg:
                # Return the message, value pair
                return [new_msg, msg_val]
            else:
                return None

    def graph_analog(self, msg=None, new_data=None):
        """Plot data from the Arduino using the analog_data and analog_plot objects"""
        print('plotting data... ' + str(new_data))
        if msg is None or new_data is None:
            msg, new_data = self.read_serial()
        if new_data is not None:
            data = [int(new_data)]
            self.analog_data.add(data)
            self.analog_plot.update(self.analog_data)
            # self.arduino.flush()


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


def arduino_logging_loop(target_arduino, do_graph=True, do_log=True, update_delay=0.25):
    """Loop on an arduino connection and log data from it"""
    logger = CsvLogger()
    while True:
        try:
            # Get the serial message from the Arduino
            line = target_arduino.read_serial()
            if line is not None:
                msg, new_data = line
                data = [int(new_data)]
                if do_log and new_data is not None:
                    if len(data) >= 1:
                        ns_data = data[0]
                        logger.update(ns_data)
                if do_graph and new_data is not None:
                    target_arduino.graph_analog(data)
            time.sleep(update_delay)
        except KeyboardInterrupt:
            # The user has stopped the loop. Clean up the connection
            print('exiting')
            break
    # close serial
    target_arduino.close()


if __name__ == '__main__':
    # Set up an Arduino
    str_port = 'COM9'
    # str_port = '/dev/tty.usbserial-A7006Yqh'  # <--- How to do this in Linux (see list_serial_ports)
    led_arduino = ArduinoInterface(port=str_port, baudrate=9600)  # Set up an Arduino

    str_port = 'COM11'
    ping_arduino = ArduinoInterface(port=str_port, baudrate=9600)

    # Run in a loop and log values from the arduino
    # arduino_logging_loop(ping_arduino, update_delay=0.1)

    # Run in a loop and pass user-specified values to the Arduino
    # arduino_control(led_arduino)

    # Control one arduino with another
    arduino_to_arduino(ping_arduino, led_arduino)
