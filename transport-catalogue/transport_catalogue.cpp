#include "transport_catalogue.h"
#include <string_view>

namespace utils {
    string FormatNumber(size_t n) {
        return to_string(n);
    }
    
    string NotFound(string_view key, string_view name) {
        return string(key) + " " + string(name) + ": not found\n";
    }
    
    string Success(string_view key, string_view name, string_view info) {
        return string(key) + " " + string(name) + ": " + string(info) + "\n";
    }
    
    template<typename Map, typename Key>
    bool Exists(const Map& map, const Key& key) {
        return map.find(key) != map.end();
    }
}


// Private
double TransportCatalogue::ComputeRouteDistance(const vector<string_view>& route) const {
    double distance = 0.0;
    
    for (size_t i = 1; i < route.size(); ++i) {
        const Coordinates& from = stops_coordinates_.at(route[i - 1]);
        const Coordinates& to = stops_coordinates_.at(route[i]);
        distance += ComputeDistance(from, to);
    }
    
    return distance;
}

const string_view TransportCatalogue::GetStoredString(string_view name) {
    auto [it, inserted] = unique_names_.emplace(name);
    return string_view(*it);
}

const string TransportCatalogue::FormatRouteInfo(string_view bus_id) {
    bus_id = GetStoredString(bus_id);

    const vector<string_view>& route = buses_routes_.at(bus_id);
    const unordered_set<string_view> unique_stops(route.begin(), route.end());
    
    if (distance_cache_.find(bus_id) == distance_cache_.end()) {
        distance_cache_[GetStoredString(bus_id)] = ComputeRouteDistance(route);
    }

    return (
        to_string(route.size()) + " stops on route, " + 
        to_string(unique_stops.size()) + " unique stops, " + 
        to_string(distance_cache_[bus_id]) + " route length"
    );
}

const string TransportCatalogue::FormatStopInfo(string_view stop_name) {
    stop_name = GetStoredString(stop_name);
    set<string_view> &stops_routes = stops_routes_.at(stop_name); 
    string buses = stops_routes.size() ? "buses " : "no buses";

    for (const string_view bus : stops_routes) {
        buses +=  string(bus) + " ";
    }
    return buses;
} 


// Public
TransportCatalogue::TransportCatalogue() {}
TransportCatalogue::~TransportCatalogue() {}

void TransportCatalogue::AddStop(std::string_view name, Coordinates coordinates) {
    std::string_view stored_name = GetStoredString(name);
    stops_coordinates_[stored_name] = coordinates;
    stops_routes_.try_emplace(stored_name);
}

const Coordinates* TransportCatalogue::FindStop(string_view name) const {
    auto it = stops_coordinates_.find(name);
    return it != stops_coordinates_.end() ? &it->second : nullptr;
}

const string TransportCatalogue::GetStopInfo(string_view stop_name) {
    stop_name = GetStoredString(stop_name);
    
    using namespace utils;

    return (
        !Exists(stops_routes_, stop_name)
        ? 
        NotFound(STOP_KEY, stop_name) : Success(STOP_KEY, stop_name, FormatStopInfo(stop_name))
    );
}

void TransportCatalogue::AddBus(string_view id, const vector<string_view>& route) {
    string_view stored_id = GetStoredString(id);
    
    vector<string_view> stored_route;
    stored_route.reserve(route.size());
    
    for (string_view stop_name : route) {
        string_view stored_stop = GetStoredString(stop_name);
        stored_route.push_back(stored_stop);
    }
    
    buses_routes_[stored_id] = std::move(stored_route);

    for (const string_view stop : buses_routes_.at(stored_id)) {
        stops_routes_[stop].emplace(stored_id);
    }
}

const vector<string_view>* TransportCatalogue::FindBus(string_view id) const {
    auto it = buses_routes_.find(id);
    return it != buses_routes_.end() ? &it->second : nullptr;
}

const string TransportCatalogue::GetBusInfo(string_view bus_id) {
    bus_id = GetStoredString(bus_id);

    using namespace utils;

    return (
        !Exists(buses_routes_, bus_id)
        ? 
        NotFound(BUS_KEY, bus_id) : Success(BUS_KEY, bus_id, FormatRouteInfo(bus_id))
    );
}