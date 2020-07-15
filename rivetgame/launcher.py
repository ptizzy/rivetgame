#!/usr/bin/env python
"""

Tools for interfacing with Arduinos using Python

"""

import os
import platform
import time

import serial
import glob
import struct
import pygame
from pygame.locals import *


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
        # try:
        if self.interface is None or not isinstance(self.interface, serial.Serial):
            self.interface = serial.Serial(self.port, self.baudrate)
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
        print("LINE:", line)
        if line is not None:
            new_msg = line[0]
            try:
                new_msg = chr(new_msg)
            except ValueError:
                return {}

            msg_val = line[1:]
            try:
                msg_val = int(msg_val)
            except ValueError:
                return {}
            print(new_msg, msg_val)

            if target_msg is not None and new_msg != target_msg:
                return {}
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

    def get_state(self):
        return self.msg_dict.get("S", 0)

    def get_points(self, player_num=0):
        if player_num == 0:
            return self.msg_dict.get("P", 0)
        elif player_num == 1:
            return self.msg_dict.get("p", 0)


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
        return glob.glob('/dev/ttyS*') + glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')


def main(arduino):
    pygame.init()

    # "Initializes a new pygame screen using the framebuffer"
    # Based on "Python GUI in Linux frame buffer"
    # http://www.karoltomala.com/blog/?p=679
    disp_no = os.getenv("DISPLAY")
    if disp_no:
        print("I'm running under X display = {0}".format(disp_no))

    # Check which frame buffer drivers are available
    # Start with fbcon since directfb hangs with composite output
    drivers = ['fbcon', 'directfb', 'svgalib']
    found = False
    for driver in drivers:
        # Make sure that SDL_VIDEODRIVER is set
        if not os.getenv('SDL_VIDEODRIVER'):
            os.putenv('SDL_VIDEODRIVER', driver)
        try:
            pygame.display.init()
        except pygame.error:
            print('Driver: {0} failed.'.format(driver))
            continue
        found = True
        break

    if not found:
        raise Exception('No suitable video driver found!')

    size = (pygame.display.Info().current_w, pygame.display.Info().current_h)
    print("Framebuffer size: %d x %d" % (size[0], size[1]))
    screen = pygame.display.set_mode(size, pygame.FULLSCREEN)
    # Clear the screen to start
    screen.fill((255, 255, 255))
    # Initialise font support
    pygame.font.init()

    font = pygame.font.SysFont(None, 32)
    # Render the screen
    pygame.display.update()

    # Run until the user asks to quit
    counter = 0
    running = True
    while running:

        # Did the user press escape? If so, exit to the console
        for event in pygame.event.get():
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False

        # Check for updated messages from the Arduino
        if counter % 300 == 0:
            arduino.read_serial()

        # Fill the background with white
        screen.fill((255, 255, 255))

        # Draw a solid blue circle in the center
        pygame.draw.circle(screen, (0, 0, 255), (350, 350), 75)

        mode = arduino.get_state()
        if mode == 0:
            demo_screen(arduino, screen, font)
        else:
            game_screen(arduino, screen, font)

        img = font.render('Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
        screen.blit(img, (20, 20))
        img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
        screen.blit(img, (20, 80))
        img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
        screen.blit(img, (20, 180))

        # Flip the display
        pygame.display.flip()

    # Done! Time to quit.
    pygame.quit()


def demo_screen(arduino, screen, font):
    img = font.render('DEMO Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
    screen.blit(img, (20, 20))
    img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
    screen.blit(img, (20, 80))
    img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
    screen.blit(img, (20, 180))


def game_screen(arduino, screen, font):
    img = font.render('GAME Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
    screen.blit(img, (20, 20))
    img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
    screen.blit(img, (20, 80))
    img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
    screen.blit(img, (20, 180))

def run(controller_port):
    try:
        # Manage the connection to the arduino with the context manager
        with ArduinoInterface(controller_port, baudrate=115200) as arduino:
            print("arduino.read_serial()", arduino.read_serial())
            main(arduino)
    except Exception as e:
        print("Exception", e)


if __name__ == '__main__':
    while True:
        run('COM1')
        run('COM2')
        run('COM3')
        run('COM4')
        run('COM5')
        run('COM6')
        run('/dev/ttyUSB0')
        run('/dev/ttyUSB1')
        run('/dev/ttyUSB2')
        run('/dev/ttyUSB3')
        time.sleep(5)
