#include "SimuCore/SimuCoreTick.hpp"
#include "SimuCore/generated/Config.hpp"
#include <memory>
#include <chrono>
#include <thread>

class SimuCoreTickNative : public SimuCoreTick
{
public:
    SimuCoreTickNative() = default;
    void wait_for_next_tick() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_in_ms));
    }
};

// Factory
std::unique_ptr<SimuCoreTick> SimuCoreTick::create()
{
    return std::make_unique<SimuCoreTickNative>();
}
