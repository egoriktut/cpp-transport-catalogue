#pragma once
#include <cmath>
#include <cstddef>
#include <deque>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <set>
#include "geo.h"


struct Stop {
    const static constexpr std::string_view key = "Stop";
    std::string name;
    Coordinates coordinates;
};

struct  Route {
    std::vector<const Stop*> stops;
    std::optional<double> route_distance;
    std::optional<double> curvature;
    std::optional<size_t> unique_stops;
    std::optional<size_t> stops_on_route;
};

struct Bus {
    const static constexpr std::string_view key = "Bus";
    std::string id;
    Route route;
};


class TransportCatalogue {
private:
    std::unordered_set<std::string> unique_stops_names_;
    std::unordered_set<std::string> unique_stop_to_stop_names_;
    std::unordered_set<std::string> unique_buses_names_;

    std::unordered_map<std::string_view, Stop> stops_;
    std::unordered_map<std::string_view, Bus> buses_;  
    std::unordered_map<std::string_view, unsigned> stop_to_stop_;
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stop_buses_;

private:
    const std::string_view GetStoredStopString(std::string_view name);
    const std::string_view GetStoredBusString(std::string_view name);
    const std::string_view GetStoredStopToStopString(std::string_view start, std::string_view end);
    void ComputeRouteDistance(Route* route);
    void ComputeUniqueStops(Route* route);

public:

    TransportCatalogue();
    ~TransportCatalogue();

    void AddStop(std::string_view name, Distance distance);
    const Stop* FindStop(std::string_view name);
    const std::unordered_set<const Bus*> GetStopBuses(const Stop* stop);

    void AddBus(std::string_view id, const std::vector<std::string_view>& route);
    const Bus* FindBus(std::string_view id);
    const Route* GetRoute(std::string_view id);
};