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
    double route_distance;
    double curvature;
    size_t unique_stops;
    size_t stops_on_route;
};

struct Bus {
    const static constexpr std::string_view key = "Bus";
    std::string id;
    std::vector<const Stop*> stops;
};

struct StopPairHash {
    size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
        return std::hash<const void*>{}(p.first) ^ (std::hash<const void*>{}(p.second) << 1);
    }
};


class TransportCatalogue {
private:
    std::deque<Stop> stops_storage_;
    std::deque<Bus> bus_storage_;
    std::unordered_map<std::string_view, Stop*> stops_;
    std::unordered_map<std::string_view, Bus*> buses_;

    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> distances_;
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stop_buses_;

private:
    void ComputeRouteDistance(const Bus* bus, Route& route);
    void ComputeUniqueStops(const Bus* bus, Route& route);

public:
    TransportCatalogue();
    ~TransportCatalogue();

    void SetDistanceToStop(std::string_view name, const std::vector<std::pair<std::string, unsigned>>& distance);
    void SetDistance(std::string_view from, std::string_view to, double distance);
    std::optional<double> GetDistance(std::string_view from, std::string_view to);

    void AddStop(std::string_view name, Coordinates coordinates);
    const Stop* FindStop(std::string_view name);
    const std::unordered_set<const Bus*> GetStopBuses(const Stop* stop);

    void AddBus(std::string_view id, const std::vector<std::string_view>& route);
    const Bus* FindBus(std::string_view id);
    const Route GetRoute(std::string_view id);
};