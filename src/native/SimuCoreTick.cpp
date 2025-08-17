#include "SimuCore/SimuCoreTick.hpp"
#include "SimuCore/SimuCoreConfig.hpp"
#include <memory>
#include <chrono>
#include <thread>

class SimuCoreTickNative : public SimuCoreTick {
public:
    SimuCoreTickNative(const SimuCoreConfig& cfg)
        : tick_ms_(cfg.tick_ms) {
            
        }

    void wait_for_next_tick() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms_));
    }
    // Factory
    std::unique_ptr<SimuCoreTick> create(const SimuCoreConfig& cfg) {
        return std::make_unique<SimuCoreTickNative>(cfg);
    }


private:
    unsigned tick_ms_;
};

