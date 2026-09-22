#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
    const static constexpr std::string_view key = "Stop";
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    const static constexpr std::string_view key = "Bus";
    std::string id;
    std::vector<const Stop*> stops;
    std::vector<const Stop*> endpoints;
    bool is_roundtrip = false;
};

struct Route {
    double route_distance = 0.0;
    double curvature = 0.0;
    size_t unique_stops = 0;
    size_t stops_on_route = 0;
};

struct BusStat {
    int stop_count = 0;
    int unique_stop_count = 0;
    int route_length = 0;
    double curvature = 0.0;
};

struct StopPairHash {
    size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
        return std::hash<const void*>{}(p.first) ^ (std::hash<const void*>{}(p.second) << 1);
    }
};

}  // namespace domain