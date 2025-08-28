import asyncio
import websockets
import json
from simucore_schema import SimuCoreProtocol


class SimuCoreSystem:
    def __init__(self, uri="ws://localhost:8080"):
        self._uri = uri
        self._component_dump = {}
    async def connect(self):
        async with websockets.connect(self._uri) as ws:
            self._component_dump = await ws.recv()
            print(self._component_dump)

            while True:
                data = await ws.recv()
                print(data)
                # bulk_update = SimuCoreProtocol.model_validate(json.loads(data))
                # print(bulk_update)
                # bulk_update.values[0].value = "0"
                # await ws.send(bulk_update.model_dump_json())


async def main():
    simucore_system = SimuCoreSystem()
    simucore_system.connect()

asyncio.run(SimuCoreSystem().connect())
