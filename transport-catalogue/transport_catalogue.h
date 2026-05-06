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
    std::string_view name;
    Coordinates coordinates;
};

struct  Route {
    std::vector<const Stop*> stops;
    double route_distance = .0;
    size_t unique_stops = 0;
};

struct Bus {
    const static constexpr std::string_view key = "Bus";
    std::string_view id;
    /*
    Как вариант думаю можно сделать отдельную мапу для route в TransportCatalogue
    с ключем bus_id. Но мне кажется это уже будет лишнее нагромождение, 
    и лучше маршрут закреплять за автобусом в структуре. По сути одно и тоже - вкусовщина
    */
    Route route;
};

struct BusView {
    BusView(const Bus* bus);
    
    const size_t stops_on_route;
    const size_t unique_stops;
    const double route_distance;
};

class TransportCatalogue {
private:
    std::unordered_set<std::string> unique_names_;

    std::unordered_map<std::string_view, Stop> stops_;
    std::unordered_map<std::string_view, Bus> buses_;  
    std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stop_buses_;

private:
    const std::string_view GetStoredString(std::string_view name);
    void ComputeRouteDistance(Bus& bus);
    void ComputeUniqueStops(Bus& bus);

public:

    TransportCatalogue();
    ~TransportCatalogue();

    void ComputeBusesInfo();

    void AddStop(std::string_view name, Coordinates coordinates);
    const Stop* FindStop(std::string_view name);
    const std::unordered_set<const Bus*> GetStopBuses(const Stop* stop);

    void AddBus(std::string_view id, const std::vector<std::string_view>& route);
    const Bus* FindBus(std::string_view id);
    const BusView GetBusInfo(const Bus* bus);
};