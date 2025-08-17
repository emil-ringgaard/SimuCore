#pragma once

#include <string>
#include <vector>

struct SimuCoreConfig_Boot {
    int lol;
};

struct SimuCoreConfig {
    int tick_ms;
    bool log_enabled;
    SimuCoreConfig_Boot boot;
};