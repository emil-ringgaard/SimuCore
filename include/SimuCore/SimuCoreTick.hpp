#pragma once
#include <SimuCore/generated/Config.hpp>
#include <memory>

class SimuCoreTick
{
public:
    SimuCoreTick() : _sleep_in_ms(1000u / SimuCore::config.sample_frequency.getValue())
    {
    }
    virtual ~SimuCoreTick() = default;

    // Called once per loop to enforce timing
    virtual void wait_for_next_tick() = 0;
    static std::unique_ptr<SimuCoreTick> create(); // internally picks platform-specific impl
protected:
    unsigned _sleep_in_ms;
};
