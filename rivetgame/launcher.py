#!/usr/bin/env python
"""

Tools for interfacing with Arduinos using Python

"""

import os
import platform
import pygame
from pygame.locals import *
from rivetgame.arduino import ArduinoInterface


def main():
    pygame.init()
    # screen = pygame.display.set_mode((1280, 720), pygame.HWSURFACE | pygame.DOUBLEBUF)

    # Set up the drawing window
    # screen = pygame.display.set_mode([500, 500])

    # "Ininitializes a new pygame screen using the framebuffer"
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
    # Render the screen
    pygame.display.update()

    # Run until the user asks to quit
    running = True
    while running:

        # Did the user press escape?
        for event in pygame.event.get():
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE:
                    running = False

        # Fill the background with white
        screen.fill((255, 255, 255))

        # Draw a solid blue circle in the center
        pygame.draw.circle(screen, (0, 0, 255), (250, 250), 75)

        # Flip the display
        pygame.display.flip()

    # Done! Time to quit.
    pygame.quit()


if __name__ == '__main__':

    controller_port = None
    system_name = platform.system()
    if system_name == "Windows":
        controller_port = "COM3"
    else:
        # Assume Linux or something else
        controller_port = '/dev/ttyACM0'

    # Manage the connection to the arduino with the context manager
    with ArduinoInterface(controller_port, baudrate=115200) as arduino:
        print("arduino.read_serial()", arduino.read_serial())
        main()
