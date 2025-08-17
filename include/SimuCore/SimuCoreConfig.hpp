#pragma once

struct SimuCoreConfig {
    unsigned tick_ms = 100;     // update period
    unsigned max_iterations = 0; // 0 = run forever
    bool verbose = false;
};
