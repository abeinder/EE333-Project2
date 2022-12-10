import asyncio
from bleak import BleakScanner


async def get_device(name: str):
    devices = await BleakScanner.discover()
    for d in devices:
        if d.name == name:
            return d


async def main():
    d = await get_device("BT05")

    print("-----------------")
    print(d)
    print(d.metadata['uuids'])
    print()

asyncio.run(main())
