#include "SimuCore/SimuCoreTick.hpp"
#include "SimuCore/SimuCoreBaseConfig.hpp"
#include <memory>
#include <Arduino.h>

class SimuCoreTickArduino : public SimuCoreTick {
public:
    explicit SimuCoreTickArduino() = default;

    void wait_for_next_tick() override {
        delay(_sleep_in_ms); // Arduino handles platform-specific delay
    }
};

std::unique_ptr<SimuCoreTick> SimuCoreTick::create() {
    return std::make_unique<SimuCoreTickArduino>();
}