#include "transport_catalogue.h"
#include <string>

using namespace std;

// Bus
void Bus::ComputeRouteDistance() {
    for (size_t i = 1; i < stops.size(); ++i) {
        const Stop& from = *stops[i - 1];
        const Stop& to = *stops[i];
        route_distance += ComputeDistance(from.coordinates, to.coordinates);
    }
}

void Bus::ComputeUniqueStops() {
    unordered_set<const Stop*> unique_stops_(stops.begin(), stops.end()); 
    unique_stops = unique_stops_.size();
}

BusView::BusView(const Bus* const bus)
    : stops_on_route(to_string(bus->stops.size()))
    , unique_stops(to_string(bus->unique_stops))
    , route_distance(to_string(bus->route_distance))
{}

// TransportCatalogue
// Private
const string_view TransportCatalogue::GetStoredString(string_view name) {
    auto [it, inserted] = unique_names_.emplace(name);
    return string_view(*it);
}

// Public
TransportCatalogue::TransportCatalogue() {}
TransportCatalogue::~TransportCatalogue() {}

void TransportCatalogue::ComputeBusesInfo() {
    for (auto &[bus_id, bus] : buses_) {
        bus.ComputeRouteDistance();
        bus.ComputeUniqueStops();
    }
}

void TransportCatalogue::AddStop(string_view name, Coordinates coordinates) {
    string_view stored_name = GetStoredString(name);
    
    auto it = stops_.find(stored_name);
    if (it != stops_.end()) {
        it->second.coordinates = coordinates;
        return;
    }
    
    Stop stop;
    stop.name = stored_name;
    stop.coordinates = coordinates;
    stops_.try_emplace(stored_name, std::move(stop));
}

const Stop* TransportCatalogue::FindStop(string_view name) {
    name = GetStoredString(name);
    auto it = stops_.find(name);
    return it != stops_.end() ? &it->second : nullptr;
}
vector<const Bus*> TransportCatalogue::GetStopBuses(const Stop* const stop) {
    auto it = stop_buses_.find(stop->name);
    if (it == stop_buses_.end()) {
        return {};
    }
    return {it->second.begin(), it->second.end()};
}

void TransportCatalogue::AddBus(string_view id, const vector<string_view>& route) {
    string_view stored_id = GetStoredString(id);
    
    Bus bus;
    bus.id = stored_id;
    bus.stops.reserve(route.size());

    for (string_view stop_name : route) {
        string_view stored_stop = GetStoredString(stop_name);
        
        // Если остановка еще не добавлена, создаем ее для валидного указателя
        if (stops_.find(stored_stop) == stops_.end()) {
            AddStop(stored_stop, {0.0, 0.0});
        }
        
        Stop* stop_ptr = &stops_.at(stored_stop);
        bus.stops.push_back(stop_ptr);
    }
    buses_.try_emplace(stored_id, std::move(bus));
    
    const Bus* bus_ptr = &buses_.at(stored_id);
    for (const Stop* stop : bus_ptr->stops) {
        stop_buses_[stop->name].insert(bus_ptr);
    }
}

const Bus* TransportCatalogue::FindBus(string_view id) {
    id = GetStoredString(id);
    auto it = buses_.find(id);
    return it != buses_.end() ? &it->second : nullptr;
}

const BusView TransportCatalogue::GetBusInfo(const Bus* const bus) {
    return BusView(bus);
}