#pragma once
#include <memory>

class SimuCoreHAL {
public:
    virtual ~SimuCoreHAL() = default;

    virtual void init() = 0;
    virtual void update() = 0;

    static std::unique_ptr<SimuCoreHAL> create(); // internally picks platform-specific impl
};
