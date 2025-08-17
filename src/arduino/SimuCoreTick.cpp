#include "SimuCore/SimuCoreTick.hpp"
#include "SimuCore/SimuCoreConfig.hpp"
#include <memory>
#include <Arduino.h>

class SimuCoreTickArduino : public SimuCoreTick {
public:
    explicit SimuCoreTickArduino(const SimuCoreConfig& cfg)
        : tick_ms_(cfg.tick_ms) {}

    void wait_for_next_tick() override {
        delay(tick_ms_); // Arduino handles platform-specific delay
    }

private:
    unsigned tick_ms_;
};
