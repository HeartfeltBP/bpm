import asyncio
# from bleak import BleakScanner, BleakClient, BleakError
from bleak import *
# basic client to simulate the external device that will read
# BLE values from Arduino device (named ActiveBP)

async def connectArduinoCLI():
    devices = await BleakScanner.discover()
    i = 0
    for d in devices:
        if(i > 100):
                break
        print(i, getattr(d, 'name'), getattr(d, 'metadata'))
        i += 1
    print('Select device to connect to')
    select = input('Select Device #:')
    return devices[select]


async def connectArduino():
    devices = await BleakScanner.discover()
    for d in devices:
       if d.name == 'bpm':
           async with BleakClient(d) as client:
               print('Connected')
               return client

def readCharacteristic(client: BleakClient):
    if client:
        print('💦')
        services = client.services
        print(services.characteristics)
        # services.get_characteristic()

async def main():
    client = await connectArduino()
    if (client == None):
        client = await connectArduinoCLI()
        while(client):
            readCharacteristic(client)


asyncio.run(main())