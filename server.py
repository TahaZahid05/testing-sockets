import asyncio
import websockets

# Set to keep track of all connected clients
connected_clients = set()
message_history = []

async def send_pings(websocket):
    while True:
        try:
            await websocket.ping()
            await asyncio.sleep(5)
        except:
            break

async def echo(websocket):  # Add 'path' argument
    # Add client to the set
    connected_clients.add(websocket)
    asyncio.create_task(send_pings(websocket))
    print(f"New client connected. Total clients: {len(connected_clients)}")

    for old_message in message_history:
        await websocket.send(old_message)

    try:
        async for message in websocket:
            print(f"Received: {message}")
            message_history.append(message)

            # Broadcast the received message to all connected clients
            for client in connected_clients:
                if client != websocket:
                    await client.send(message)
                    

    except websockets.exceptions.ConnectionClosed:
        print("Client disconnected.")

    finally:
        # Remove the client from the set on disconnect
        connected_clients.remove(websocket)
        print(f"Client removed. Total clients: {len(connected_clients)}")

async def main():
    server = await websockets.serve(echo, "0.0.0.0", 12345)  # Accept connections from any device
    print("WebSocket Server Started on ws://0.0.0.0:12345")
    await server.wait_closed()  # Keeps the server running

if __name__ == "__main__":
    asyncio.run(main())
    
