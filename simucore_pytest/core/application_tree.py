# noqa: N815
from pydantic import BaseModel


class ConnectedInput(BaseModel):
    id: int

class Parameter(BaseModel):
    id: int
    name: str
    typeName: str
    value: str

class Output(BaseModel):
    id: int
    name: str
    typeName: str
    value: str
    connectedInputs: list[ConnectedInput] | None = []

class Input(BaseModel):
    id: int
    name: str
    typeName: str
    value: str

class PhysicalInput(Input):
    pass

class PhysicalOutput(Output):
    pass

class Component(BaseModel):
    id: int
    name: str
    Components: list[Component] | None = []
    Inputs: list[Input] | None = []
    Outputs: list[Output] | None = []
    PhysicalOutputs: list[PhysicalOutput] | None = None
    PhysicalInputs: list[PhysicalInput] | None = None
    Parameters: list[Parameter] | None = []

Component.model_rebuild()

class ApplicationTree(BaseModel):
    id: int
    name: str
    Components: list[Component]
    config: dict = {}
