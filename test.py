import asyncio
import websockets

async def simple_client():
    uri = "ws://localhost:8080"
    
    async with websockets.connect(uri) as websocket:
        # Send a message
        await websocket.send("Hello from Python!")
        
        # Receive response
        response = await websocket.recv()
        print(f"Server said: {response}")

# Run it
asyncio.run(simple_client())