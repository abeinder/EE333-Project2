"""
This is the primary application for the flight controller. In order to connect
to the bluetooth, the address and UUID for the breakout board must be found.
Several that we have used on our project are listed below. These can be found
by using the listAll.py program within Testing.

"""

import sys
import pygame
from pygame.locals import *
import asyncio
import time
from bleak import BleakClient

pygame.init()
pygame.display.set_caption('game base')
# screen = pygame.display.set_mode((500, 500), 0, 32)
clock = pygame.time.Clock()

pygame.joystick.init()
joysticks = [pygame.joystick.Joystick(i) for i in range(pygame.joystick.get_count())]
for joystick in joysticks:
    print(joystick.get_name())

my_square = pygame.Rect(50, 50, 50, 50)
my_square_color = 0
colors = [(255, 0, 0), (0, 255, 0), (0, 0, 255)]
motion = [0, 0, 0, 0]
accumulated = [0, 0, 0, 0]
increase = False
decrease = False
count = 0


# address = "F0:45:DA:03:1A:2B"
# address = "0C:B2:B7:2D:B3:AF"
address = "F0:45:DA:03:34:30"
# MODEL_NBR_UUID = "00004124-0000-1000-8000-00805f9b34fb"
MODEL_NBR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"


def get_new_accumulated():

    global increase
    global decrease
    global count

    if abs(motion[0]) < 0.05:
        motion[0] = 0
    if abs(motion[1]) < 0.05:
        motion[1] = 0
    if abs(motion[2]) < 0.05:
        motion[2] = 0
    if abs(motion[3]) < 0.05:
        motion[3] = 0

    for event in pygame.event.get():
        if event.type == JOYAXISMOTION:
            if event.axis < 4:
                motion[event.axis] = -event.value * 20
            elif event.axis == 4 and event.value > 0:
                print("Decrease")
                decrease = True
            elif event.axis == 5 and event.value > 0:
                print("Increase")
                increase = True
            elif event.axis == 4 and event.value < 0:
                print("Stop decreasing")
                decrease = False
            elif event.axis == 5 and event.value < 0:
                print("Stop increasing")
                increase = False
        if event.type == QUIT:
            pygame.quit()
            sys.exit()

    count += 1

    for i in range(0, len(motion)):
        accumulated[i] += motion[i]
        if accumulated[i] < 7:
            accumulated[i] = 7
        if accumulated[i] >= 127:
            accumulated[i] = 126
        accumulated[i] = int(accumulated[i])

    clock.tick(60)


async def read_characteristic(client, uuid):
    model_number = await client.read_gatt_char(uuid)
    return str("{0}".format("".join(map(chr, model_number))))


async def write_characteristic(client, uuid, data: str):
    encoded_data = data.encode()
    response = await client.write_gatt_char(uuid, bytearray(encoded_data))
    return response


async def main(address):
    client = BleakClient(address)

    try:
        await client.connect()
        while True:
            model_number = await read_characteristic(client, MODEL_NBR_UUID)
            print(f"Characteristic: {model_number}")
            get_new_accumulated()
            speed_val = 0
            if increase:
                speed_val = 1
            elif decrease:
                speed_val = 2

            first_rot = accumulated[1]
            second_rot = accumulated[3]

            first7 = first_rot >> 2
            second7 = second_rot >> 2
            register = (speed_val << 13) + (first7 << 7) + (second7 << 1)
            val1 = register >> 8
            val2 = (register - ((register >> 8) << 8)) >> 1
            char1 = chr(val1)
            char2 = chr(val2)

            value = ((char1 + char2 +
                      "                                          ")[0:20])

            await write_characteristic(client, MODEL_NBR_UUID, value)

            print("--------------------")
            print(value)
            print("{0:b}".format(register))
            print(accumulated[1])
            print(accumulated[3])
            print()
            time.sleep(0.1)
    except Exception as e:
        print(e)
    finally:
        await client.disconnect()

asyncio.run(main(address))


quit()
