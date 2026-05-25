import json
from pydantic import TypeAdapter
from tenacity import retry, stop_after_attempt, wait_exponential, retry_if_exception_type
from websockets.exceptions import WebSocketException
from websockets import ClientConnection
from simucore_pytest.core.application_tree import ApplicationTree
from simucore_pytest.core.schemas import StartSimulation, TickSystem, ApplicationTreeData, ApplicationInfo, ApplicationInfoProtocol, Response, UpdatePysicalInputsProtocol, UpdateInput
import time
from websockets.sync.client import connect


_response_list_adapter = TypeAdapter(Response)

class SimuCoreSystem:
    def __init__(self, uri: str = "ws://localhost:8080"):
        self._uri = uri
        self._ws: ClientConnection | None = None
        self.application_tree: ApplicationTree

    @retry(
        stop=stop_after_attempt(5),
        wait=wait_exponential(multiplier=1, min=1, max=10),
        retry=retry_if_exception_type(
            (ConnectionRefusedError, OSError, WebSocketException)
        ),
    )
    def start(self) -> None:
        if not self._ws:
            self._ws = connect(self._uri)
            application_tree_json = json.loads(self._ws.recv())
        else:
            application_tree_json = json.loads(self.get_application_tree())
        self.application_tree = ApplicationTree(**application_tree_json)
        self._ws.send(StartSimulation().model_dump_json())
        response_json = self._ws.recv()
        resp = _response_list_adapter.validate_json(response_json)
        assert resp.status == "SUCCESS"

    def update_value(self, id: int, value: str) -> None:
        ws = self._require_ws()
        ws.send(
            UpdatePysicalInputsProtocol(
                parameters=[UpdateInput(id=id, value=value)]
            ).model_dump_json()
        )
        ws.recv()
    
    def get_application_info(self) -> ApplicationInfo:
        ws = self._require_ws()
        ws.send(ApplicationInfo().model_dump_json())
        return ws.recv()

    def get_application_tree(self) -> ApplicationTree:
        ws = self._require_ws()
        ws.send(ApplicationTreeData().model_dump_json())
        return ws.recv()

    def tick(self, number_of_ticks: int) -> None:
        ws = self._require_ws()
        ws.send(TickSystem(number_of_ticks=number_of_ticks).model_dump_json())
        ws.recv()

    def close(self) -> None:
        if self._ws is not None:
            try:
                self._ws.close()
            finally:
                self._ws = None

    def _require_ws(self) -> ClientConnection:
        if self._ws is None:
            raise RuntimeError("SimuCoreSystem.start() has not been called")
        return self._ws