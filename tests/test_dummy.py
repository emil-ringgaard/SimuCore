from simucore_pytest.core.simulation import SimuCoreSystem

def test_dummy(simulation_instance: SimuCoreSystem) -> None:
    print(simulation_instance.application_tree)
