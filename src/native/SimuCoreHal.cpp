#include "SimuCore/SimuCoreHAL.hpp"
#include <iostream>


class NativeHAL : public SimuCoreHAL {
public:
    void init() override {
        std::cout << "Native HAL init" << std::endl;
    }
    void update() override {

    }
    std::unique_ptr<SimuCoreHAL> create() {
        return std::make_unique<NativeHAL>();
    }
};

