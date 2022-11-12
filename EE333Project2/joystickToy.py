import sys

import pygame

from pygame.locals import *
pygame.init()
pygame.display.set_caption('game base')
screen = pygame.display.set_mode((500, 500), 0, 32)
clock = pygame.time.Clock()

pygame.joystick.init()
joysticks = [pygame.joystick.Joystick(i) for i in range(pygame.joystick.get_count())]
for joystick in joysticks:
    print(joystick.get_name())

my_square = pygame.Rect(50, 50, 50, 50)
my_square_color = 0
colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
motion = [0, 0]
accumulated = [0, 0]

def get_new_accumulated():

    if abs(motion[0]) < 0.1:
        motion[0] = 0
    if abs(motion[1]) < 0.1:
        motion[1] = 0

    for event in pygame.event.get():
        if event.type == JOYAXISMOTION:
            print(event)
            if event.axis < 2:
                motion[event.axis] = event.value
        if event.type == QUIT:
            pygame.quit()
            sys.exit()

    print(motion)

    for i in range(0, len(motion)):
        accumulated[i] += motion[i]
        if accumulated[i] < 0:
            accumulated[i] = 0
        if accumulated[i] > 255:
            accumulated[i] = 255
        accumulated[i] = int(accumulated[i])

    print(accumulated)

    clock.tick(60)

while True:

    screen.fill((0, 0, 0))

    pygame.draw.rect(screen, colors[my_square_color], my_square)
    if abs(motion[0]) < 0.1:
        motion[0] = 0
    if abs(motion[1]) < 0.1:
        motion[1] = 0
    my_square.x += motion[0] * 10
    my_square.y += motion[1] * 10

    get_new_accumulated()

    pygame.display.update()
    clock.tick(60)
