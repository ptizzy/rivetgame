#!/usr/bin/env python
"""

Tools for interfacing with Arduinos using Python

"""

import pygame
from arduino import ArduinoInterface


def main():
    pygame.init()
    screen = pygame.display.set_mode((1280, 720), pygame.HWSURFACE | pygame.DOUBLEBUF)

    # Set up the drawing window
    # screen = pygame.display.set_mode([500, 500])

    # Run until the user asks to quit
    running = True
    while running:

        # Did the user click the window close button?
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
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

    # Manage the connection to the arduino with the context manager
    with ArduinoInterface("COM3", baudrate=115200) as arduino:
        print("arduino.read_serial()", arduino.read_serial())
        main()
