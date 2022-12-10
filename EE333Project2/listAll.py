import asyncio
from bleak import BleakScanner
from bleak import BleakClient


async def get_device(name: str):
    devices = await BleakScanner.discover()
    for d in devices:
        if d.name == name:
            return d


async def main():
    # devices = await BleakScanner.discover()

    d = await get_device("BT05")

    ble_device = BleakClient

    print(d)

asyncio.run(main())
