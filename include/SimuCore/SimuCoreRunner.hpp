// SimuCoreRunner.hpp
#pragma once
#include "SimuCore/SimuCoreApplication.hpp"

class SimuCoreRunner {
public:
    SimuCoreRunner(SimuCoreApplication& app, unsigned tick_ms)
        : app_(app), tick_ms_(tick_ms) {}

    void runTick() {
        app_.update();
    }

    unsigned tick_ms() const { return tick_ms_; }

private:
    SimuCoreApplication& app_;
    unsigned tick_ms_;
};
