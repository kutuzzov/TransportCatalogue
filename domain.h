#pragma once

#include "geo.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>

namespace domain {

    struct Stop {
        Stop(const std::string& name, const geo::Coordinates& coordinates);
        int GetDistance(Stop* to);

        std::string name;
        geo::Coordinates coordinates;
        std::unordered_map<std::string_view, int> stop_distances;
    };

    struct Bus {
        Bus(const std::string& name, std::vector<Stop*> stops, bool is_circle);

        std::string name;
        std::vector<Stop*> stops;
        bool is_circle;
        Stop* final_stop = nullptr;
    };

} //namespace domain