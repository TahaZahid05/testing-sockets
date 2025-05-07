import asyncio
import websockets
import string

connected_clients = set()
message_history = []

assigned_ids = {}  
available_ids = list(string.ascii_uppercase)  # ['A', 'B', ..., 'Z']

async def echo(websocket):
    connected_clients.add(websocket)

    if available_ids:
        client_id = available_ids.pop(0)
    else:
        client_id = '?'  

    assigned_ids[websocket] = client_id

    print(f"New client connected: ID {client_id}. Total clients: {len(connected_clients)}")

    await websocket.send(f"[Server] You are Client ID: {client_id}")

    for old_message in message_history:
        await websocket.send(old_message)

    try:
        async for message in websocket:
            print(f"Received from {client_id}: {message}")
            message_history.append(message)

            for client in connected_clients:
                if client != websocket:
                    await client.send(message)

    except websockets.exceptions.ConnectionClosed:
        print(f"Client {client_id} disconnected.")

    finally:
        connected_clients.remove(websocket)

        removed_id = assigned_ids.pop(websocket, None)
        if removed_id and removed_id != '?':
            available_ids.append(removed_id)
            available_ids.sort()  

        print(f"Client {client_id} removed. Total clients: {len(connected_clients)}")

async def main():
    server = await websockets.serve(echo, "0.0.0.0", 12345)
    print("WebSocket Server Started on ws://0.0.0.0:12345")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())
