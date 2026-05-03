#pragma once
#include <cmath>
#include <cstddef>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <set>
#include "geo.h"

using namespace std;

constexpr string_view BUS_KEY = "Bus";
constexpr string_view STOP_KEY = "Stop";

class TransportCatalogue {
private:
    unordered_set<string> unique_names_;
    
    unordered_map<string_view, Coordinates> stops_coordinates_;
    unordered_map<string_view, set<string_view>> stops_routes_;
    unordered_map<string_view, vector<string_view>> buses_routes_;
    unordered_map<string_view, double> distance_cache_;
    

private:
    
    double ComputeRouteDistance(const vector<string_view>& route) const;
    const string_view GetStoredString(string_view name);
    const string FormatRouteInfo(string_view bus_id);
    const string FormatStopInfo(string_view stop_name);

public:

    TransportCatalogue();
    ~TransportCatalogue();

    void AddStop(string_view name, Coordinates coordinates);
    const Coordinates* FindStop(string_view name) const;
    const string GetStopInfo(string_view stop_name);

    void AddBus(string_view id, const vector<string_view>& route);
    const vector<string_view>* FindBus(string_view id) const;
    const string GetBusInfo(string_view bus_id);
};