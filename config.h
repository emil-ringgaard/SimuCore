#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

struct Config {
    int tick_ms;
    bool log_enabled;

    inline void from_json(const nlohmann::json& j) {
        j.at("tick_ms").get_to(tick_ms);
        j.at("log_enabled").get_to(log_enabled);
    }
    inline nlohmann::json to_json() const {
        nlohmann::json j;
        j["tick_ms"] = tick_ms;
        j["log_enabled"] = log_enabled;
        return j;
    }
};


Config h;