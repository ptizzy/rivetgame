import pygame


def demo_screen(arduino, screen, font):
    # Fill the background with white
    screen.fill((255, 255, 255))

    # Draw a solid blue circle in the center
    pygame.draw.circle(screen, (0, 0, 255), (350, 350), 75)

    img = font.render('DEMO Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
    screen.blit(img, (20, 20))
    img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
    screen.blit(img, (20, 80))
    img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
    screen.blit(img, (20, 180))


def game_screen(arduino, screen, font):
    # Fill the background with white
    screen.fill((255, 255, 255))

    # Draw a solid blue circle in the center
    pygame.draw.circle(screen, (0, 0, 255), (350, 350), 75)

    img = font.render('GAME Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
    screen.blit(img, (20, 20))
    img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
    screen.blit(img, (20, 80))
    img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
    screen.blit(img, (20, 180))