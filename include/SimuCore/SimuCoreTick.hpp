#pragma once
#include <SimuCore/SimuCoreConfig.hpp>
#include <memory>

class SimuCoreTick {
public:
    virtual ~SimuCoreTick() = default;

    // Called once per loop to enforce timing
    virtual void wait_for_next_tick() = 0;
    static std::unique_ptr<SimuCoreTick> create(std::unique_ptr<SimuCoreConfig> &config); // internally picks platform-specific impl
};
