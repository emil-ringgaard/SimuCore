from collections.abc import Generator
import json
from pathlib import Path
import subprocess

from platformio.public import load_build_metadata
from platformio.run.cli import cli as run_cli
import pytest

from simucore_pytest.config.models import SimucoreTestConfig
from simucore_pytest.core.simulation import SimuCoreSystem


class SimucorePytestConfig(pytest.Config): # pyright: ignore[reportGeneralTypeIssues]
    simucore_test_config: SimucoreTestConfig


class SimucorePytestSession(pytest.Session): # pyright: ignore[reportGeneralTypeIssues]
    config: SimucorePytestConfig # pyright: ignore[reportIncompatibleVariableOverride]


def pytest_configure(config: SimucorePytestConfig) -> None:
    test_config = Path.cwd() / "test_config.json"
    config.simucore_test_config = SimucoreTestConfig(**json.loads(test_config.read_text()))


def pytest_sessionstart(session: SimucorePytestSession) -> None:
    platformio_project_path = session.config.simucore_test_config.platform_io_project_path.resolve()
    run_cli(["-d", platformio_project_path, "-e", "native", "-t", "fullclean", "-s"], standalone_mode=False)
    run_cli(["-d", platformio_project_path, "-e", "native"], standalone_mode=False)


@pytest.fixture(scope="session")
def simulation_instance_session(request: SimucorePytestSession) -> Generator[SimuCoreSystem]:
    platformio_project_path = request.config.simucore_test_config.platform_io_project_path.resolve()
    meta = load_build_metadata(platformio_project_path, ["native"])
    assert meta
    prog_path = meta["native"]["prog_path"]
    process = subprocess.Popen([prog_path])
    try:
        simucore_system = SimuCoreSystem()
        yield simucore_system
    finally:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()

@pytest.fixture
def simulation_instance(simulation_instance_session: SimuCoreSystem) -> SimuCoreSystem:
    simulation_instance_session.start()
    return simulation_instance_session
