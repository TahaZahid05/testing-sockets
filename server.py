import asyncio
import websockets
import json
import string

connected_clients = set()
files = {}  # Dict to store files

# Broadcast a message to all clients
async def broadcast(message):
    disconnected = set()
    for client in connected_clients:
        try:
            await client.send(message)
        except websockets.exceptions.ConnectionClosed:
            disconnected.add(client)
    
    for client in disconnected:
        connected_clients.remove(client)

# Handle incoming messages
async def handle_message(ws, message):
    try:
        data = json.loads(message)
    except json.JSONDecodeError:
        return  # Ignore invalid JSON

    if data["type"] == "list_files":
        await ws.send(json.dumps({
            "type": "file_list",
            "files": list(files.keys())
        }))

    elif data["type"] == "request_file":
        if data["filename"] in files:
            await ws.send(json.dumps({
                "type": "file_content",
                "filename": data["filename"],
                "content": files[data["filename"]]
            }))

    elif data["type"] == "save_file":
        files[data["filename"]] = data["content"]
        await broadcast(json.dumps({
            "type": "file_updated",
            "filename": data["filename"]
        }))

# Main connection handler
# async def echo(websocket):
#     connected_clients.add(websocket)
#     print(f"New client connected. Total clients: {len(connected_clients)}")

#     try:
#         async for message in websocket:
#             print(f"Received: {message}")
#             await handle_message(websocket, message)
#     except websockets.exceptions.ConnectionClosed:
#         print("Client disconnected.")
#     finally:
#         connected_clients.remove(websocket)
#         print(f"Client removed. Total clients: {len(connected_clients)}")
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
            await handle_message(websocket, message)
            # Broadcast the message to all other clients
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
        if(len(connected_clients) == 0):
            message_history.clear()

# Start the server
async def main():
    server = await websockets.serve(echo, "0.0.0.0", 12345)
    print("WebSocket Server Started on ws://0.0.0.0:12345")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())
