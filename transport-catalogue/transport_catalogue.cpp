#include "transport_catalogue.h"
#include "geo.h"

using namespace std;

// TransportCatalogue
// Private
void TransportCatalogue::ComputeRouteDistance(const Bus* bus, Route& route) {
    double distance = .0;
    double real_distance = .0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        const Stop& from = *bus->stops[i - 1];
        const Stop& to = *bus->stops[i];
        const double distance_between = ComputeDistance(from.coordinates, to.coordinates);
        const optional<double> real_distance_between = GetDistance(from.name, to.name);

        distance += distance_between;
        real_distance += real_distance_between.value_or(distance_between);
    }
    route.route_distance = real_distance;
    route.curvature = real_distance / distance;
}

void TransportCatalogue::ComputeUniqueStops(const Bus* bus, Route& route) {
    unordered_set<const Stop*> unique_stops_(bus->stops.begin(), bus->stops.end()); 
    route.unique_stops = unique_stops_.size();
}

// Public
TransportCatalogue::TransportCatalogue() {}
TransportCatalogue::~TransportCatalogue() {}

void TransportCatalogue::SetDistanceToStop(
    std::string_view name, 
    const std::vector<std::pair<std::string, unsigned>>& distance
) {
    for (const auto &[stop_name, real_distance] : distance) {
        SetDistance(name, stop_name, real_distance);
    }
}

void TransportCatalogue::SetDistance(string_view from, string_view to, double distance) {
    const Stop* stop_from = FindStop(from);
    const Stop* stop_to = FindStop(to);

    if (stop_from == nullptr || stop_to == nullptr) {
        return;
    }
    distances_[{stop_from, stop_to}] = distance;
}

optional<double> TransportCatalogue::GetDistance(string_view from, string_view to) {
    const Stop* stop_from = FindStop(from);
    const Stop* stop_to = FindStop(to);

    if (stop_from == nullptr || stop_to == nullptr) {
        return nullopt;
    }
    
    auto it = distances_.find({stop_from, stop_to});
    it = (
        it == distances_.end()
        ?
        distances_.find({stop_to, stop_from}) : it
    );
    if (it != distances_.end()) {
        return it->second;
    }
    return nullopt;
}

void TransportCatalogue::AddStop(string_view name, Coordinates coordinates) {
    Stop stop;
    stop.name = string(name);
    stop.coordinates = coordinates;

    stops_storage_.push_back(std::move(stop));
    const auto& stop_el = &stops_storage_.back();
    stops_.try_emplace(stop_el->name, stop_el);
}

const Stop* TransportCatalogue::FindStop(string_view name) {
    auto it = stops_.find(name);
    return it != stops_.end() ? it->second : nullptr;
}

const unordered_set<const Bus*> TransportCatalogue::GetStopBuses(const Stop* stop) {
    auto it = stop_buses_.find(stop->name);
    if (it == stop_buses_.end()) {
        return {};
    }
    return it->second;
}

void TransportCatalogue::AddBus(string_view id, const vector<string_view>& route) {
    Bus bus;
    bus.id = string(id);
    bus.stops.reserve(route.size());

    for (string_view stop_name : route) {
        /* 
        Если остановка еще не добавлена, создаем ее для валидного указателя.
        Оставляю на всякий случай, хоть и обработал эту ситуацию в input_reader. 
        Думаю будет полнезно для публичного метода
        */ 
        if (stops_.find(stop_name) == stops_.end()) {
            AddStop(stop_name, {0.0, 0.0});
        }
        
        const Stop* stop_ptr = stops_.at(stop_name);
        bus.stops.push_back(stop_ptr);
    }

    bus_storage_.push_back(std::move(bus));
    const auto& bus_el = &bus_storage_.back();
    buses_.try_emplace(bus_el->id, bus_el);
    
    const Bus* bus_ptr = buses_.at(id);
    for (const Stop* stop : bus_ptr->stops) {
        stop_buses_[stop->name].insert(bus_ptr);
    }
}

const Bus* TransportCatalogue::FindBus(string_view id) {
    auto it = buses_.find(id);
    return it != buses_.end() ? it->second : nullptr;
}

const Route TransportCatalogue::GetRoute(string_view id) {
    auto it = buses_.find(id);
    if (it == buses_.end()) {
        return {};
    }
    const Bus* bus = it->second;
    Route route;

    ComputeRouteDistance(bus, route);
    ComputeUniqueStops(bus, route);
    route.stops_on_route = bus->stops.size();
    return route;
}