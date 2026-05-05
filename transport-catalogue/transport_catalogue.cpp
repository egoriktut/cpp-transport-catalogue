#include "transport_catalogue.h"

using namespace std;

// Bus
double Bus::GetDistance() {
    if (!route_distance.has_value()) {
        double distance_ = 0.0;
        for (size_t i = 1; i < stops.size(); ++i) {
            const Stop& from = *stops[i - 1];
            const Stop& to = *stops[i];
            distance_ += ComputeDistance(from.coordinates, to.coordinates);
        }
        route_distance = distance_;
    }
    return route_distance.value();
}

// TransportCatalogue
// Private
const string_view TransportCatalogue::GetStoredString(string_view name) {
    auto [it, inserted] = unique_names_.emplace(name);
    return string_view(*it);
}

// Public
TransportCatalogue::TransportCatalogue() {}
TransportCatalogue::~TransportCatalogue() {}

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

Stop* TransportCatalogue::FindStop(string_view name) {
    name = GetStoredString(name);
    auto it = stops_.find(name);
    return it != stops_.end() ? &it->second : nullptr;
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
    
    Bus* bus_ptr = &buses_.at(stored_id);
    for (Stop* stop : bus_ptr->stops) {
        stop->buses.insert(bus_ptr);
    }
}

Bus* TransportCatalogue::FindBus(string_view id) {
    id = GetStoredString(id);
    auto it = buses_.find(id);
    return it != buses_.end() ? &it->second : nullptr;
}