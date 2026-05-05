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

struct Bus;

struct Stop {
    const static constexpr std::string_view key = "Stop";
    std::string_view name;
    Coordinates coordinates;
    std::unordered_set<Bus*> buses;
};

struct Bus {
    const static constexpr std::string_view key = "Bus";
    std::string_view id;
    std::vector<Stop*> stops;
    std::optional<double> route_distance;

    double GetDistance();
};

class TransportCatalogue {
private:
    std::unordered_set<std::string> unique_names_;
    // Не вижу смысла в deque, если можно хранить удобно в мапах 
    std::unordered_map<std::string_view, Stop> stops_;
    std::unordered_map<std::string_view, Bus> buses_;

private:
    const std::string_view GetStoredString(std::string_view name);

public:

    TransportCatalogue();
    ~TransportCatalogue();

    void AddStop(std::string_view name, Coordinates coordinates);
    Stop* FindStop(std::string_view name);

    void AddBus(std::string_view id, const std::vector<std::string_view>& route);
    Bus* FindBus(std::string_view id);
};