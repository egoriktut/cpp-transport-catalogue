#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
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

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct RouteItemWait {
    static constexpr std::string_view key = "Wait";
    std::string stop_name;
    double time = 0.0;
};

struct RouteItemBus {
    static constexpr std::string_view key = "Bus";
    std::string bus;
    int span_count = 0;
    double time = 0.0;
};

using RouteItem = std::variant<RouteItemWait, RouteItemBus>;

struct RouteResult {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

}  // namespace domain