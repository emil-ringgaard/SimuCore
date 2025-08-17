#pragma once
#include "SimuCore/SimuCoreHAL.hpp"
#include <SimuCore/SimuCoreTick.hpp>
#include <SimuCore/SimuCoreConfig.hpp>

class SimuCoreApplication {
public:
    SimuCoreApplication(SimuCoreConfig &config) :
    simu_core_config(&config),
    hal(SimuCoreHAL::create())
    {
        simu_core_tick = SimuCoreTick::create(simu_core_config);
        hal->init();
    }

    virtual ~SimuCoreApplication() = default;

    // The user implements their application logic here
    virtual void updateLogic() = 0;

    // Call this every tick
    void update() {
        hal->update();      // HAL update
        updateLogic();      // User application logic
        simu_core_tick->wait_for_next_tick();
    }

protected:
    std::unique_ptr<SimuCoreHAL> hal;
    std::unique_ptr<SimuCoreTick> simu_core_tick;
    std::unique_ptr<SimuCoreConfig> simu_core_config;
};
