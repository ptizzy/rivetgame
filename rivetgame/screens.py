import pygame

from resource.fwa_named_colors import *


def demo_screen(arduino, screen, time):

    draw_rivetrace_bkg(arduino, screen, time, "Learn How")

    text_w_drop( screen, 'Pick up a rivet gun to play', screen.get_width()*0.5, 240, 60, (255,255,255), 5, 100 )

    gun_image=pygame.image.load("graphics/rivet_gun.png")
    screen.blit(gun_image, (screen.get_width()*0.25-gun_image.get_width()*0.5,screen.get_height()*0.4) )
    screen.blit(pygame.transform.flip(gun_image, 1, 0), (screen.get_width()*0.75-gun_image.get_width()*0.5,screen.get_height()*0.4) )

def training_screen(arduino, screen, time):

    draw_rivetrace_bkg(arduino, screen, time, "Instructions")

    column_y=screen.get_height()*0.2

    text_w_drop( screen, 'Place the end of your gun over a red rivet hole', screen.get_width()*0.5, column_y, 50, (255,255,255), 5, 100 )

    column_x=screen.get_width()*0.3

    draw_box(screen, column_x, column_y+220, 400, 300 )
    draw_box(screen, screen.get_width()-column_x, column_y+220, 400, 300 )

    column_y=screen.get_height()*0.6

    text_w_drop( screen, 'Orient the gun perpendicular to the metal sheet', screen.get_width()*0.5, column_y, 50, (255,255,255), 5, 100 )

    draw_box(screen, column_x, column_y+220, 400, 300 )
    draw_box(screen, screen.get_width()-column_x, column_y+220, 400, 300 )

def draw_box(screen, x_center, y_center, x_width, y_width ):

    pygame.draw.rect(screen, fwa_medium_blue, (x_center-x_width*0.5,y_center-y_width*0.5,x_width,y_width))
    pygame.draw.rect(screen, fwa_2nd_teal_dk, (x_center-x_width*0.5,y_center-y_width*0.5,x_width,y_width), 5)



def game_screen(arduino, screen, time):

    draw_rivetrace_bkg(arduino, screen, time, "RIVET RACE!")

    text_w_drop( screen, 'Set as many rivets as you can before time runs out!', screen.get_width()*0.5, 240, 50, (255,255,255), 5, 100 )

    # left player score display

    column_x=screen.get_width()*0.22
    column_y=screen.get_height()*0.35

    pygame.draw.rect(screen, fwa_2nd_teal_dk, (0,column_y-20,screen.get_width()*0.35,360), 5)
    pygame.draw.rect(screen, fwa_medium_blue, (0,column_y-20,screen.get_width()*0.35,360))

    text_w_drop( screen, 'Left Player', column_x, column_y, 60, (255,255,255), 5 )
    text_w_drop( screen, 'Rivets set:', column_x, column_y+100, 60, left_player_color, 5 )
    text_w_drop( screen, str(arduino.get_points(player_num=0)), column_x, column_y+190, 140, left_player_color, 10 )

    # right player score display

    column_x=screen.get_width()*0.78

    pygame.draw.rect(screen, fwa_2nd_teal_dk, (screen.get_width()*0.65,column_y-20,screen.get_width()*0.35,360), 5)
    pygame.draw.rect(screen, fwa_medium_blue, (screen.get_width()*0.65,column_y-20,screen.get_width()*0.35,360))

    text_w_drop( screen, 'Right Player', column_x, column_y, 60, (255,255,255), 5 )
    text_w_drop( screen, 'Rivets set:', column_x, column_y+100, 60, right_player_color, 5 )
    text_w_drop( screen, str(arduino.get_points(player_num=1)), column_x, column_y+190, 140, right_player_color, 10 )

    # countdown

    text_w_drop( screen, ':24', screen.get_width()*0.5, screen.get_height()*0.65, 300, (255,255,255), 15 )
    text_w_drop( screen, 'Time Remaining', screen.get_width()*0.5, screen.get_height()*0.92, 50, (255,255,255), 5 )


def training_complete_screen(arduino, screen, time):
    # Fill the background with white
    screen.fill((255, 5, 255))
    font = pygame.font.Font("fonts/Helvetica35Thin_22435.ttf", 35)

    # Draw a solid blue circle in the center
    print(time)
    pygame.draw.circle(screen, (0, 0, 200), (int((time * 100) % 300), 350), 75)

    img = font.render('Training Complete Mode: {}'.format(arduino.get_state()), True, (0, 0, 0))
    screen.blit(img, (20, 20))
    img = font.render('Player 1 Points: {}'.format(arduino.get_points(player_num=0)), True, (0, 0, 0))
    screen.blit(img, (20, 80))
    img = font.render('Player 2 Points: {}'.format(arduino.get_points(player_num=1)), True, (0, 0, 0))
    screen.blit(img, (20, 180))

def draw_rivetrace_bkg(arduino, screen, time, screen_title):

    bkg_image=pygame.image.load("graphics/interface_bkg.png")
    screen.fill((0,0,0))
    screen.blit(bkg_image,(0,0))

    text_w_drop( screen, screen_title, screen.get_width()*0.5, 50, 140, (255,196,51), 10 )

    text_w_drop( screen, str(arduino.get_state()), 20, 20, 20, (255,196,51), 2, 50 )  # print state for debugging

    text_w_drop( screen, 'v1.0', 20, screen.get_height()-20, 20, (255,196,51), 2, 50 )  # print version number

def text_w_drop( screen, text_string, x, y, size, text_color, drop_dist, drop_opacity=None ):

    font = pygame.font.Font("fonts/Politica XT Bold.ttf", size)

    if drop_opacity is None:
        drop_opacity=140

    img = font.render(text_string, True, (0,0,0))
    alpha_img = pygame.Surface(img.get_size(), pygame.SRCALPHA) #creates a temp surface
    alpha_img.fill((255,255,255,drop_opacity)) # fills temp surface w alpha
    img.blit(alpha_img, (0,0), special_flags=pygame.BLEND_RGBA_MULT) # blits with original text surface
    screen.blit(img, (x-img.get_width()*0.5 + drop_dist, y + drop_dist)) # drop shadow blit

    img = font.render(text_string, True, text_color)
    img.set_alpha(100)
    screen.blit(img, (x-img.get_width()*0.5, y))
