import json
from pydantic import TypeAdapter
from tenacity import retry, stop_after_attempt, wait_exponential, retry_if_exception_type
from websockets.exceptions import WebSocketException
from websockets import ClientConnection
from simucore_pytest.core.application_tree import ApplicationTree
from simucore_pytest.core.schemas import StartSimulation, TickSystem, SubscribePayload, SubscribeProtocol, ApplicationInfoProtocol, UpdatePysicalInputsProtocol, UpdateInput
import time
from websockets.sync.client import connect


_response_list_adapter = TypeAdapter(list[ApplicationInfoProtocol])

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
        # `connect()` returns a ClientConnection; using it without `with`
        # is fine, we just have to close it ourselves in `close()`.
        self._ws = connect(self._uri)

        application_tree_json = json.loads(self._ws.recv())
        self.application_tree = ApplicationTree(**application_tree_json)

        self._ws.send(StartSimulation().model_dump_json())
        resp = _response_list_adapter.validate_json(self._ws.recv())
        assert len(resp)
        assert resp[0].response.status == "SUCCESS"

    def update_value(self, id: int, value: str) -> None:
        ws = self._require_ws()
        ws.send(
            UpdatePysicalInputsProtocol(
                parameters=[UpdateInput(id=id, value=value)]
            ).model_dump_json()
        )
        ws.recv()

    def tick(self, number_of_ticks: int) -> None:
        ws = self._require_ws()
        for _ in range(number_of_ticks):
            ws.send(TickSystem().model_dump_json())
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