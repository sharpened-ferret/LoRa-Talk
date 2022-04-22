import asyncio
from contextlib import nullcontext
import websockets
import json
import sqlite3

from websockets import serve

connected = set()

DATABASE_PATH = "chat.db"

current_msg = ""

# async def handler(websocket, path):
#     connected.add(websocket)
#     # print(connected)
#     async for message in websocket:
#         print(message)
#         await recieve_msg(message)
#         response = message
#         # websockets.broadcast(connected, response)
#         for client in connected:
#             try:
#                 await client.send(response)
#                 print("Send message to {}".format(client))
#             except:
#                 print("Failed to Send message to {}".format(client))

async def handler(websocket, path):
    global connected
    if (websocket not in connected):
        connected.add(websocket)

        print("New Connection: {}".format(str(websocket)))
        try:
            while True:
                reciever_task = asyncio.ensure_future(websocket.recv())
                sender_task = asyncio.ensure_future(send_msg(websocket, path))
                done, pending = await asyncio.wait(
                    [reciever_task, sender_task],
                    return_when=asyncio.FIRST_COMPLETED
                )

                if reciever_task in done:
                    message = reciever_task.result()
                    await message_echo(message[2:])
                    await recieve_msg(message[2:])
                else:
                    reciever_task.cancel()

                if sender_task in done:
                    message = sender_task.result()
                    #await send_msg(websocket, path)
                    if (message != None):
                        print(message)
                        await websocket.send(str(message))
                else:
                    print("Cancelled Send")
                    sender_task.cancel()
                    
        finally:
            print("Websocket Disconnected: " + str(websocket))
            connected.remove(websocket)

async def send_msg(websocket, path):
    global current_msg
    if (current_msg != ""):
        await websocket.send(current_msg)
        returnMessage = current_msg
        current_msg = ""
        return returnMessage



async def recieve_msg(message):
    global current_msg
    print("Recieved: " + message)
    current_msg = message
    data = json.loads(message)

    cur = con.cursor()
    cur.execute("INSERT INTO messages(username, timestamp, message) VALUES (?, ?, ?)", (data["username"], data["timestamp"], data["message"]))
    con.commit()


async def socket_manager():
    async with serve(handler, "0.0.0.0", 80):
        await asyncio.Future()

async def message_echo(message):
    print("Starting Echo Service")
    websockets.broadcast(connected, message)


def main():
    global con
    con = sqlite3.connect(DATABASE_PATH)
    
    asyncio.create_task(asyncio.run(socket_manager()))
    print("Test")

main()