#pragma once

#include <string>
#include <vector>

struct SimuCoreBaseConfig_Boot_Boot2 {
    int test1;
    std::vector<std::string> test2;
};

struct SimuCoreBaseConfig_Boot {
    SimuCoreBaseConfig_Boot_Boot2 boot2;
};

struct SimuCoreBaseConfig {
    int sample_frequency;
    bool log_enabled;
    bool enable_webserver;
    SimuCoreBaseConfig_Boot boot;
};

const SimuCoreBaseConfig config_instance;