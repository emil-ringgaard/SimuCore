from simucore_pytest.core.simulation import SimuCoreSystem


def test_dummy(simulation_instance: SimuCoreSystem) -> None:
    simulation_instance.update_value(id=2608728740, value="123453")
    simulation_instance.tick(20000)
    assert simulation_instance.get_application_info().up_time_in_milli_seconds == 200000
