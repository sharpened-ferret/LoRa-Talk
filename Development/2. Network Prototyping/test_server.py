import asyncio
from matplotlib.pyplot import connect
import websockets
import json
import sqlite3

connected = set()
DATABASE_PATH = "./chat.db"

async def process_message(message):
    # try:
        if message[0] == "0" and message[1] == 0:
            message = message[2:]
        data = json.loads(message)

        cur = con.cursor()
        cur.execute("INSERT INTO messages(username, timestamp, message) VALUES (?, ?, ?)", (data["username"], data["timestamp"], data["message"]))
        con.commit()

        return str(data)
    # except:
    #     return ""


async def message_handler(websocket):
    global connected
    connected.add(websocket)
    print("Gateway Connected: {}".format(str(websocket.local_address)))
    async for message in websocket:
        return_msg = await process_message(message)
        websockets.broadcast(connected, return_msg)

    try:
        await websocket.wait_closed()
    finally:
        print("Gateway Disconnected: " + str(websocket.local_address))
        connected.remove(websocket)

async def main():
    global con
    con = sqlite3.connect(DATABASE_PATH)

    async with websockets.serve(message_handler, "0.0.0.0", 80):
        await asyncio.Future()


asyncio.run(main())