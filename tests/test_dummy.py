from simucore_pytest.core.simulation import SimuCoreSystem

def test_dummy(simulation_instance: SimuCoreSystem) -> None:
    simulation_instance.update_value(id=2608728740, value="123453")
    simulation_instance.tick(20000)
    print(simulation_instance.get_application_info())

# def test_dummy2(simulation_instance: SimuCoreSystem) -> None:
#     simulation_instance.update_value(id=2608728740, value="123453")
#     simulation_instance.tick()
#     print(simulation_instance.get_application_info())