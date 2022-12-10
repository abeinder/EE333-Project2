"""
This script lists the address, name and UUID's of all bluetooth devices within
range.
"""

import asyncio
from bleak import BleakScanner


async def main():
    devices = await BleakScanner.discover()
    for d in devices:
        print("-----------------")
        print(d)
        print(d.metadata['uuids'])
        print()

asyncio.run(main())
