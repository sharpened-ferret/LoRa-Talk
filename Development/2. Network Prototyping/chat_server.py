import asyncio
import websockets
import json
import sqlite3

from websockets import serve

connected = set()

DATABASE_PATH = "chat.db"

async def handler(websocket):
    # connected.add(websocket)
    # print(connected)
    async for message in websocket:
        print(message)
        await recieve_msg(message)
        response = message
        # websockets.broadcast(connected, response)
        await websocket.send(response)

async def recieve_msg(message):
    data = json.loads(message)

    cur = con.cursor()
    cur.execute("INSERT INTO messages(username, timestamp, message) VALUES (?, ?, ?)", (data["username"], data["timestamp"], data["message"]))
    con.commit()

async def socket_manager():
    async with serve(handler, "localhost", 8765):
        await asyncio.Future()


def main():
    global con
    con = sqlite3.connect(DATABASE_PATH)
    
    asyncio.create_task(asyncio.run(socket_manager()))
    print("Test")

main()