import json
from pathlib import Path
import shutil
from typing import Literal

from pydantic import BaseModel

COMMANDS = Literal[
    "UPDATE_PARAMETERS",
    "SUBSCRIBE",
    "UPDATE_PHYSICAL_INPUT",
    "START_SIMULATION",
    "STOP_SIMULATION",
    "TICK",
    "INFO",
    "APPLICATION_TREE"
]
ResponseStatus = Literal["SUCCESS", "FAILURE", "WARNING"]


class UpdateBase(BaseModel):
    id: int
    value: str


class UpdateInput(UpdateBase):
    pass


class UpdateParameter(UpdateBase):
    pass


class SubscribePayload(BaseModel):
    id: int
    value: str
    type: str


class SubscribeProtocol(BaseModel):
    command: COMMANDS = "SUBSCRIBE"
    payload: list[SubscribePayload]


class UpdataParametersProtocol(BaseModel):
    command: COMMANDS = "UPDATE_PARAMETERS"
    parameters: list[UpdateParameter]


class UpdatePysicalInputsProtocol(BaseModel):
    command: COMMANDS = "UPDATE_PHYSICAL_INPUT"
    parameters: list[UpdateInput]


class TickSystem(BaseModel):
    command: COMMANDS = "TICK"
    number_of_ticks: int


class StartSimulation(BaseModel):
    command: COMMANDS = "START_SIMULATION"


class StopSimulation(BaseModel):
    command: COMMANDS = "STOP_SIMULATION"

class ApplicationInfo(BaseModel):
    command: COMMANDS = "INFO"

class ApplicationTreeData(BaseModel):
    command: COMMANDS = "APPLICATION_TREE"


class Response(BaseModel):
    status: ResponseStatus
    message: str


class ApplicationInfoProtocol(BaseModel):
    response: Response
    up_time_in_milli_seconds: int
    subscribed_signals: list[SubscribePayload]


class Config(BaseModel):
    sample_frequency: float = 100
    log_enabled: bool = False
    enable_webserver: bool = True
    blah: str


class SimulationModelConfig(BaseModel):
    module_path: str
    model_name: str


def get_library_root(env):
    lib_json = Path(__file__).parent.parent.parent / "library.json"
    lib_name = json.loads(lib_json.read_text())["name"]
    return Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV") / lib_name


def generate_simcore_schema(env, pydantic_model: type[BaseModel]):
    name = pydantic_model.__name__
    generated_folder = get_library_root(env) / "scripts" / "generated"
    schema_path = generated_folder.joinpath(name + ".schema.json")
    generated_folder.mkdir(exist_ok=True)
    with open(schema_path, "w") as schema:
        json.dump(pydantic_model.model_json_schema(), schema)
    return {"name": name, "schema_path": schema_path}


def generate_simucore_schemas(env):
    shutil.rmtree(get_library_root(env) / "scripts" / "generated", ignore_errors=True)
    all_schemas = [
        generate_simcore_schema(env, SubscribeProtocol),
        generate_simcore_schema(env, TickSystem),
        generate_simcore_schema(env, UpdatePysicalInputsProtocol),
        generate_simcore_schema(env, UpdataParametersProtocol),
        generate_simcore_schema(env, ApplicationInfoProtocol),
        generate_simcore_schema(env, SimulationModelConfig),
    ]
    return all_schemas


if __name__ == "__main__":
    generate_simucore_schemas(None)
