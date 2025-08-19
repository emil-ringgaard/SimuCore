#pragma once
// #if __cplusplus < 201703L
// #error "This library requires at least C++17. \
// Please add this to your platformio.ini:\
// build_unflags = -std=gnu++11 -std=gnu++14\
// build_flags = -std=gnu++17"
// #endif

#include "SimuCore/SimuCoreHAL.hpp"
#include <SimuCore/SimuCoreTick.hpp>
#include <SimuCore/SimuCoreBaseConfig.hpp>
#include <SimuCore/Component.hpp>

class SimuCoreApplication : public Component {
private:
    void update() override {
        }
    
public:
    SimuCoreApplication() :
        Component(nullptr),
        hal(SimuCoreHAL::create()),
        simu_core_tick(SimuCoreTick::create())
    {
    }
    virtual ~SimuCoreApplication() = default;

    
    void startApp() {
        hal->update();
        updateAll();
        Component::updateAllBindings();
        simu_core_tick->wait_for_next_tick();
    }

protected:
    virtual void init() = 0;
    std::unique_ptr<SimuCoreHAL> hal;
    std::unique_ptr<SimuCoreTick> simu_core_tick;
};
