import asyncio
import websockets

# Set to keep track of all connected clients
connected_clients = set()

async def echo(websocket):  # Add 'path' argument
    # Add client to the set
    connected_clients.add(websocket)
    print(f"New client connected. Total clients: {len(connected_clients)}")

    try:
        async for message in websocket:
            print(f"Received: {message}")

            # Broadcast the received message to all connected clients
            for client in connected_clients:
                if client != websocket:  # Avoid sending back to sender
                    await client.send(f"Client said: {message}")

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
