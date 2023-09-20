#!/usr/bin/env python
"""

Tools for interfacing with Arduinos using Python

"""

import os
import time

import pygame
clock = pygame.time.Clock()
from pygame.locals import *
from ArduinoInterface import ArduinoInterface
#from firebase import push_text_log
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
            # push_text_log('Driver: {0} failed.'.format(driver))
            continue
        found = True
        break

    if not found:
        raise Exception('No suitable video driver found!')

    size = (pygame.display.Info().current_w, pygame.display.Info().current_h)
    print("Framebuffer size: %d x %d" % (size[0], size[1]))
    flags = FULLSCREEN | DOUBLEBUF
    screen = pygame.display.set_mode(size, flags, 16)
    # Clear the screen to start
    screen.fill((255, 255, 255))
    # Initialise font support
    pygame.font.init()
    # Remove cursor
    pygame.mouse.set_visible(0)

    # Load images
    load_images(screen)

    # Render the screen
    pygame.display.update()

    # Run until the user asks to quit
    running = True
    current_mode = -1
    while running:
        # Did the user press escape? If so, exit to the console
        for event in pygame.event.get():
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False

        # Check for updated messages from the Arduino
        res = None
        while res is None or len(res) != 0:
            res = arduino.read_serial()

        mode = arduino.get_state()

        t = time.time() - arduino.get_start_time()

        if mode == 0:
            if mode != current_mode:
                current_mode = mode
                draw_rivetrace_bkg(arduino, screen, time, "Learn How")
                text_w_drop(screen, 'Pick up a rivet gun to play', screen.get_width() * 0.5, 240, 60, (255, 255, 255), 5, 100)
            demo_screen(arduino, screen, t)
        if mode == 1:
            training_screen(arduino, screen, t)
        if mode == 2:
            training_complete_screen(arduino, screen, t)
        if mode == 6:
            start_countdown(arduino, screen, t)
        if mode == 3:
            game_screen(arduino, screen, t)
        if mode == 4:
            game_complete_screen(arduino, screen, t)
        if mode == 5:
            leaderboard(arduino, screen, t)

        # Flip the display
        pygame.display.update()
        clock.tick(60)

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
        if "could not open port" in str(e):
            return
        # push_text_log("Exception " + str(e))


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
        run('/dev/ttyACM0')
        run('/dev/ttyACM1')
        run('/dev/ttyACM2')
        run('/dev/ttyACM3')
        time.sleep(5)
