import asyncio
from websockets import serve

async def echo(websocket):
    async for message in websocket:
        print(message)
        response = message + " OK"
        await websocket.send(response)

async def dummy_message(websocket, message):
    await websocket.send(message)

async def handler(websocket):
    echo_task = asyncio.create_task(echo(websocket))
    message_task = asyncio.create_task(dummy_message(websocket, "Server: Hi there!"))
    await echo_task
    await asyncio.sleep(5)
    # await message_task


async def main():
    async with serve(handler, "localhost", 8765):
        await asyncio.Future()

asyncio.run(main())