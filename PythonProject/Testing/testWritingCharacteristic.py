import asyncio
import time

from bleak import BleakClient

# TODO replace with your own
address = "F0:45:DA:03:1A:2B"
MODEL_NBR_UUID = "00004124-0000-1000-8000-00805f9b34fb"


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
        for i in range(0, 100):
            model_number = await read_characteristic(client, MODEL_NBR_UUID)
            print(f"Characteristic: {model_number}")
            value = (str(i) + "   ")[0:2]
            x = await write_characteristic(client, MODEL_NBR_UUID, value)
            response = await read_characteristic(client, MODEL_NBR_UUID)
            print(f"New Characteristic: {response} {i}")
            time.sleep(0.001)
    except Exception as e:
        print(e)
    finally:
        await client.disconnect()

asyncio.run(main(address))
quit()
