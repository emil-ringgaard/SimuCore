import json
import shutil
from pathlib import Path
from pydantic import BaseModel
from typing import Literal, Optional
from simucore_pytest.core.application_tree import ApplicationTree

COMMANDS = Literal[
    "UPDATE_PARAMETERS",
    "SUBSCRIBE",
    "UPDATE_PHYSICAL_INPUT",
    "START_SIMULATION",
    "STOP_SIMULATION",
    "TICK",
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


class StartSimulation(BaseModel):
    command: COMMANDS = "START_SIMULATION"


class StopSimulation(BaseModel):
    command: COMMANDS = "STOP_SIMULATION"


class Response(BaseModel):
    status: ResponseStatus
    message: str


class ApplicationInfoProtocol(BaseModel):
    response: Response
    up_time_in_s: int
    subscribed_signals: list[SubscribePayload]


class Config(BaseModel):
    sample_frequency: float = 100
    log_enabled: bool = False
    enable_webserver: bool = True
    blah: str


class SimulationModelConfig(BaseModel):
    module_path: str
    model_name: str


def generate_simcore_schema(pydantic_model: BaseModel):
    name = pydantic_model.__name__
    generated_folder = Path(__file__).parent.joinpath("generated")
    schema_path = generated_folder.joinpath(name + ".schema.json")
    generated_folder.mkdir(exist_ok=True)
    with open(schema_path, "w") as schema:
        json.dump(pydantic_model.model_json_schema(), schema)
    return {"name": name, "schema_path": schema_path}


def generate_simucore_schemas():
    shutil.rmtree(Path(__file__).parent.joinpath("generated"), ignore_errors=True)
    all_schemas = [
        generate_simcore_schema(SubscribeProtocol),
        generate_simcore_schema(UpdatePysicalInputsProtocol),
        generate_simcore_schema(UpdataParametersProtocol),
        generate_simcore_schema(ApplicationInfoProtocol),
        generate_simcore_schema(SimulationModelConfig),
    ]
    return all_schemas


if __name__ == "__main__":
    generate_simucore_schemas()
