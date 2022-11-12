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
motion = [0, 0]
accumulated = [0, 0]


address = "F0:45:DA:03:1A:2B"
MODEL_NBR_UUID = "00004124-0000-1000-8000-00805f9b34fb"


def get_new_accumulated():
    if abs(motion[0]) < 0.1:
        motion[0] = 0
    if abs(motion[1]) < 0.1:
        motion[1] = 0

    for event in pygame.event.get():
        if event.type == JOYAXISMOTION:
            if event.axis < 2:
                motion[event.axis] = event.value
        if event.type == QUIT:
            pygame.quit()
            sys.exit()

    for i in range(0, len(motion)):
        accumulated[i] += motion[i] * 10
        if accumulated[i] < 0:
            accumulated[i] = 0
        if accumulated[i] >= 255:
            accumulated[i] = 254
        accumulated[i] = int(accumulated[i])

    print(motion)
    print(accumulated)

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

    expected = "AT+\r\n"

    try:
        await client.connect()
        while True:
            model_number = await read_characteristic(client, MODEL_NBR_UUID)
            print(f"Characteristic: {model_number}")
            get_new_accumulated()
            value = ((str(accumulated[1]) + "                                    ")[0:20])

            # if model_number == expected:

            x = await write_characteristic(client, MODEL_NBR_UUID, value)
            # response = await read_characteristic(client, MODEL_NBR_UUID)
            print(f"Writing: {value}")
            time.sleep(0.001)
    except Exception as e:
        print(e)
    finally:
        await client.disconnect()

asyncio.run(main(address))


quit()
