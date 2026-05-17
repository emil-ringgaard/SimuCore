from pydantic import BaseModel
from typing import List, Optional

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
    connectedInputs: Optional[List[ConnectedInput]] = []

class Input(BaseModel):
    id: int
    name: str
    typeName: str
    value: str

class Component(BaseModel):
    id: int
    name: str
    Components: Optional[List["Component"]] = []
    Inputs: Optional[List[Input]] = []
    Outputs: Optional[List[Output]] = []
    Parameters: Optional[List[Parameter]] = []

Component.model_rebuild()

class ApplicationTree(BaseModel):
    id: int
    name: str
    Components: List[Component]
    config: Optional[dict] = []
