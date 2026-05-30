
from pydantic import BaseModel, DirectoryPath


class SimucoreTestConfig(BaseModel):
    platform_io_project_path: DirectoryPath
