
import json
import shutil
from pathlib import Path
from pydantic import BaseModel
from typing import Literal, Union

COMMANDS = Literal["UPDATE_PARAMETERS", "SUBSCRIBE", 'UPDATE_PHYSICAL_SIGNALS']

class UpdateBase(BaseModel):
    id: int
    value: str

class UpdateInput(UpdateBase):
    pass

class UpdateParameter(UpdateBase):
    pass

class SubscribePayload(BaseModel):
    id: int
    frequency: int

class SubscribeProtocol(BaseModel):
    command: COMMANDS = 'SUBSCRIBE'
    payload: list[SubscribePayload]

class UpdataParametersProtocol(BaseModel):
    command: COMMANDS = 'UPDATE_PARAMETERS'
    parameters: list[UpdateParameter]

class UpdatePysicalInputsProtocol(BaseModel):
    command: COMMANDS = 'UPDATE_PHYSICAL_SIGNALS'
    parameters: list[UpdateParameter]

class UpdatePysicalOutputsProtocol(BaseModel):
    command: COMMANDS = 'UPDATE_PHYSICAL_SIGNALS'
    parameters: list[UpdateParameter]

class Config(BaseModel):
    sample_frequency: float = 100
    log_enabled: bool = False
    enable_webserver: bool = True
    blah: str


def generate_simcore_schema(pydantic_model: BaseModel):
    name = pydantic_model.__name__
    generated_folder = Path(__file__).parent.joinpath('generated')
    schema_path = generated_folder.joinpath(name + '.schema.json')
    generated_folder.mkdir(exist_ok=True)
    with open(schema_path, 'w') as schema:
        json.dump(pydantic_model.model_json_schema(), schema)
    return {
        'name': name,
        'schema_path': schema_path
    }


def generate_simucore_schemas():
    shutil.rmtree(Path(__file__).parent.joinpath('generated'), ignore_errors=True)
    all_schemas = [generate_simcore_schema(SubscribeProtocol),
                   generate_simcore_schema(UpdatePysicalInputsProtocol)]
    return all_schemas


if __name__ == "__main__":
    generate_simucore_schemas()