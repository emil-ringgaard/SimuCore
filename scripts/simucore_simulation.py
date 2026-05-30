import json
import time

from tenacity import (
    retry,
    retry_if_exception_type,
    stop_after_attempt,
    wait_exponential,
)
from websockets.exceptions import WebSocketException

from simucore_pytest.core.application_tree import ApplicationTree
from simucore_pytest.core.schemas import (
    StartSimulation,
    SubscribePayload,
    SubscribeProtocol,
    TickSystem,
)


class SimuCoreSystem:
    def __init__(self, uri="ws://localhost:8080"):
        self._uri = uri
        self._application_tree: ApplicationTree

    @retry(
        stop=stop_after_attempt(5),
        wait=wait_exponential(multiplier=1, min=1, max=10),
        retry=retry_if_exception_type(
            (ConnectionRefusedError, OSError, WebSocketException)
        ),
    )
    async def start(self):
        import websockets

        async with websockets.connect(self._uri) as ws:
            application_tree_json = json.loads(await ws.recv())
            self._application_tree = ApplicationTree(**application_tree_json)
            await ws.send(StartSimulation().model_dump_json())
            while True:
                # Telling system to update its time. This is done to make the simulation discrete.
                await ws.send(TickSystem().model_dump_json())

    def _tick(self):
        pass

    async def _connect(self):
        import websockets

        async with websockets.connect(self._uri) as ws:
            application_tree_json = json.loads(await ws.recv())
            self._application_tree = ApplicationTree(**application_tree_json)
            print(self._application_tree)

            while True:
                # data = await ws.recv()
                # print(data)
                # bulk_update = SimuCoreProtocol.model_validate(json.loads(data))
                # print(bulk_update)
                # bulk_update.values[0].value = "0"
                # await ws.send(bulk_update.model_dump_json())
                test = SubscribeProtocol(
                    payload=[SubscribePayload(id=3621385220, value="123", type="")]
                )
                await ws.send(test.model_dump_json())
                resp = await ws.recv()
                print(resp)
                time.sleep(1)


async def main():
    simucore_system = SimuCoreSystem()
    simucore_system.connect()
