#include "transport_catalogue.h"
#include "geo.h"
#include <string>
#include <string_view>

using namespace std;

// TransportCatalogue
// Private
const string_view TransportCatalogue::GetStoredStopString(string_view name) {
    auto [it, inserted] = unique_stops_names_.emplace(name);
    return string_view(*it);
}

const string_view TransportCatalogue::GetStoredBusString(string_view name) {
    auto [it, inserted] = unique_buses_names_.emplace(name);
    return string_view(*it);
}

const string_view TransportCatalogue::GetStoredStopToStopString(string_view start, string_view end) {
    string key = string(start) + "_" + string(end);
    auto [it, inserted] = unique_stop_to_stop_names_.emplace(std::move(key));
    return *it;
}

void TransportCatalogue::ComputeRouteDistance(Route* route) {
    double distance = .0;
    double real_distance = .0;
    for (size_t i = 1; i < route->stops.size(); ++i) {
        const Stop& from = *route->stops[i - 1];
        const Stop& to = *route->stops[i];
        distance += ComputeDistance(from.coordinates, to.coordinates);

        string_view stop_to_stop_key = GetStoredStopToStopString(from.name, to.name);
        auto stop_to_stop_distance = stop_to_stop_.find(stop_to_stop_key);
        // Если не нашлось реального маршрута в одну сторону, делаем проверку на обратный 
        stop_to_stop_key = GetStoredStopToStopString(to.name, from.name);
        stop_to_stop_distance = (
            stop_to_stop_distance == stop_to_stop_.end() 
            ? 
            stop_to_stop_.find(stop_to_stop_key)
            : 
            stop_to_stop_distance
        );
        
        real_distance +=  stop_to_stop_distance != stop_to_stop_.end() ? stop_to_stop_distance->second : distance;
    }
    route->route_distance = real_distance;
    route->curvature = real_distance / distance;
}

void TransportCatalogue::ComputeUniqueStops(Route* route) {
    unordered_set<const Stop*> unique_stops_(route->stops.begin(), route->stops.end()); 
    route->unique_stops = unique_stops_.size();
}

// Public
TransportCatalogue::TransportCatalogue() {}
TransportCatalogue::~TransportCatalogue() {}

void TransportCatalogue::AddStop(string_view name, Distance distance) {
    string_view stored_name = GetStoredStopString(name);
    
    for (const auto &[stop_name, real_distance] : distance.real_distance) {
        string_view stop_to_stop_name = GetStoredStopToStopString(stored_name, stop_name);
        stop_to_stop_.try_emplace(stop_to_stop_name, real_distance);
    }

    auto it = stops_.find(stored_name);
    if (it != stops_.end()) {
        it->second.coordinates = distance.coordinates;
        return;
    }
    
    Stop stop;
    stop.name = string(stored_name);
    stop.coordinates = distance.coordinates;
    stops_.try_emplace(stored_name, std::move(stop));
}

const Stop* TransportCatalogue::FindStop(string_view name) {
    name = GetStoredStopString(name);
    auto it = stops_.find(name);
    return it != stops_.end() ? &it->second : nullptr;
}

const unordered_set<const Bus*> TransportCatalogue::GetStopBuses(const Stop* stop) {
    auto it = stop_buses_.find(stop->name);
    if (it == stop_buses_.end()) {
        return {};
    }
    return it->second;
}

void TransportCatalogue::AddBus(string_view id, const vector<string_view>& route) {
    string_view stored_id = GetStoredBusString(id);
    
    Bus bus;
    bus.id = string(stored_id);
    bus.route.stops.reserve(route.size());

    for (string_view stop_name : route) {
        string_view stored_stop = GetStoredStopString(stop_name);
        
        // Если остановка еще не добавлена, создаем ее для валидного указателя
        if (stops_.find(stored_stop) == stops_.end()) {
            AddStop(stored_stop, {{}, {0.0, 0.0}});
        }
        
        Stop* stop_ptr = &stops_.at(stored_stop);
        bus.route.stops.push_back(stop_ptr);
    }
    buses_.try_emplace(stored_id, std::move(bus));
    
    const Bus* bus_ptr = &buses_.at(stored_id);
    for (const Stop* stop : bus_ptr->route.stops) {
        stop_buses_[stop->name].insert(bus_ptr);
    }
}

const Bus* TransportCatalogue::FindBus(string_view id) {
    id = GetStoredBusString(id);
    auto it = buses_.find(id);
    return it != buses_.end() ? &it->second : nullptr;
}

const Route* TransportCatalogue::GetRoute(std::string_view id) {
    auto it = buses_.find(id);
    if (it == buses_.end()) {
        return nullptr;
    }
    Route* route = &it->second.route;

    if (!route->route_distance.has_value()) {
        ComputeRouteDistance(route);
    }
    if (!route->stops_on_route.has_value()) {
        route->stops_on_route = route->stops.size();
    }
    if (!route->unique_stops) {
        ComputeUniqueStops(route);
    }
    return route;
}