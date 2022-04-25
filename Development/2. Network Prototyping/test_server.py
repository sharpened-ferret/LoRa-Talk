import asyncio
import websockets
import json
import sqlite3

# Global variables to store session data
connected = set()
DATABASE_PATH = "./chat.db"

# -- Handles recieving and relaying messages --
# If the message is valid it is stored in the chat database, and echoed to all connected nodes. 
async def message_handler(message):
    try:
        print("Recieved Message: " + message)
        if message[0] == "0" and message[1] == "0":
            message = message[2:]
        data = json.loads(message)

        cur = con.cursor()
        cur.execute("INSERT INTO messages(username, timestamp, message) VALUES (?, ?, ?)", (data["username"], data["timestamp"], data["message"]))
        con.commit()

        return str(data)
    except:
        return ""

# -- Handles WebSocket connections from nodes --
async def socket_handler(websocket):
    global connected
    connected.add(websocket)
    print("Gateway Connected: {}".format(str(websocket.local_address)))
    
    async for message in websocket:
            return_msg = await message_handler(message)
            websockets.broadcast(connected, return_msg)

    try:
        await websocket.wait_closed()
    finally:
        print("Gateway Disconnected: " + str(websocket.local_address))
        connected.remove(websocket)

# Initialises the database connection and WebSocket server
async def main():
    global con
    con = sqlite3.connect(DATABASE_PATH)

    async with websockets.serve(socket_handler, "0.0.0.0", 80):
        await asyncio.Future()

# Starts Server
asyncio.run(main())