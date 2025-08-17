#include "SimuCore/SimuCoreHAL.hpp"
#include <memory>
#include <Arduino.h>

class ESP32HAL : public SimuCoreHAL {
public:
    void init() override {
        Serial.begin(115200);
        Serial.println("ESP32 HAL init");
    }
    void update() override {

    }
    std::unique_ptr<SimuCoreHAL> ESP32HAL::create() {
        return std::make_unique<ESP32HAL>();
    }
};


