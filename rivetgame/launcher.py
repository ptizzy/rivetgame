#!/usr/bin/env python
"""

Tools for interfacing with Arduinos using Python

"""

import os
import time

import pygame
from pygame.locals import *

from ArduinoInterface import ArduinoInterface
from screens import *


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

        mode = arduino.get_state()
        if mode == 0:
            demo_screen(arduino, screen, font)
        if mode == 2:
            training_complete_screen(arduino, screen, font)
        else:
            game_screen(arduino, screen, font)

        # Flip the display
        pygame.display.flip()

    # Done! Time to quit.
    pygame.quit()


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
