#include "request_handler.h"

#include <cmath>

using namespace std;
using namespace domain;

RequestHandler::RequestHandler(const TransportCatalogue& db)
    : db_(db) {
}

optional<BusStat> RequestHandler::GetBusStat(string_view bus_name) const {
    const Bus* bus = db_.FindBus(bus_name);
    if (bus == nullptr) {
        return nullopt;
    }

    const Route route = db_.GetRoute(bus_name);

    return BusStat{
        static_cast<int>(route.stops_on_route),
        static_cast<int>(route.unique_stops),
        static_cast<int>(std::round(route.route_distance)),
        route.curvature
    };
}

const unordered_set<const Bus*>* RequestHandler::GetBusesByStop(string_view stop_name) const {
    const Stop* stop = db_.FindStop(stop_name);
    if (stop == nullptr) {
        return nullptr;
    }

    return &db_.GetStopBuses(stop);
}