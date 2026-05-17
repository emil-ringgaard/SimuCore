import json
from pydantic import TypeAdapter
from tenacity import retry, stop_after_attempt, wait_exponential, retry_if_exception_type
from websockets.exceptions import WebSocketException
from simucore_pytest.core.application_tree import ApplicationTree
from simucore_pytest.core.schemas import StartSimulation, TickSystem, SubscribePayload, SubscribeProtocol, ApplicationInfoProtocol
import time

_response_list_adapter = TypeAdapter(list[ApplicationInfoProtocol])

class SimuCoreSystem:
    def __init__(self, uri="ws://localhost:8080"):
        self._uri = uri
        self.application_tree: ApplicationTree

    @retry(
        stop=stop_after_attempt(5),
        wait=wait_exponential(multiplier=1, min=1, max=10),
        retry=retry_if_exception_type(
            (ConnectionRefusedError, OSError, WebSocketException)
        ),
    )
    def start(self):
        from websockets.sync.client import connect

        with connect(self._uri) as ws:
            application_tree_json = json.loads(ws.recv())
            self.application_tree = ApplicationTree(**application_tree_json)
            ws.send(StartSimulation().model_dump_json())
            raw = ws.recv()
            resp: list[ApplicationInfoProtocol] = _response_list_adapter.validate_json(raw)
            assert len(resp)
            assert resp[0].response.status == "SUCCESS"
                # Telling system to update its time. This is done to make the simulation discrete.
                # await ws.send(TickSystem().model_dump_json())
