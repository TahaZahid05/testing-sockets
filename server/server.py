import asyncio
import websockets
import string

# Globals
connected_clients = set()
message_history = []

# ID tracking
assigned_ids = {}  # websocket: ID
available_ids = list(string.ascii_uppercase)  # ['A', 'B', ..., 'Z']

async def echo(websocket):
    # Add client to the set
    connected_clients.add(websocket)

    # Assign a unique client ID
    if available_ids:
        client_id = available_ids.pop(0)
    else:
        client_id = '?'  # Fallback if more than 26 clients (should not happen ideally)

    assigned_ids[websocket] = client_id

    print(f"New client connected: ID {client_id}. Total clients: {len(connected_clients)}")

    # Optionally send client their ID
    await websocket.send(f"[Server] You are Client ID: {client_id}")

    # Send message history to the new client
    for old_message in message_history:
        await websocket.send(old_message)

    try:
        async for message in websocket:
            print(f"Received from {client_id}: {message}")
            message_history.append(message)

            # Broadcast the message to all other clients
            for client in connected_clients:
                if client != websocket:
                    await client.send(f"[{client_id}] {message}")

    except websockets.exceptions.ConnectionClosed:
        print(f"Client {client_id} disconnected.")

    finally:
        # Remove client and free up their ID
        connected_clients.remove(websocket)

        removed_id = assigned_ids.pop(websocket, None)
        if removed_id and removed_id != '?':
            available_ids.append(removed_id)
            available_ids.sort()  # Keep IDs ordered for reuse

        print(f"Client {client_id} removed. Total clients: {len(connected_clients)}")

async def main():
    server = await websockets.serve(echo, "0.0.0.0", 12345)
    print("WebSocket Server Started on ws://0.0.0.0:12345")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())
