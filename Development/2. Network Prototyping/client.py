import asyncio
import time
import json
from websockets import connect

async def send_message(uri, message, username):
    display_format = "{0}: {1}"
    send_time  = time.time()

    data = {
        "username": username, 
        "timestamp": send_time,
        "message": message, 
    }

    async with connect(uri) as endpoint:

        await endpoint.send(json.dumps(data))
        print(display_format.format(username, message))
        response = await endpoint.recv()
        print(response)

def start():
    URI = "ws://localhost:8765"
    # URI = "ws://192.168.0.62:80/ws"
    welcome_string = "Welcome to LoRa Talk!"
    greeting_string = "Hi {}. Enter 'q' or 'quit' to exit. Enter anything else to send a message."
    exit_string = "Goodbye {}"

    print(welcome_string)
    username = input("Username: ")
    
    print(greeting_string.format(username))
    active = True

    while (active):
        message = input()
        if (message.lower() == "quit" or message.lower() == "q"):
            active = False
        else:
            if (message != ""):
                asyncio.run(send_message(URI, message, username))
    print(exit_string.format(username))

start()
