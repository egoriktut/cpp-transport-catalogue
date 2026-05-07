#include "stat_reader.h"
#include "transport_catalogue.h"
#include <algorithm>
#include <string>
#include <unordered_set>

using namespace std;

string NotFound(string_view key, string_view name) {
    return string(key) + " " + string(name) + ": not found\n";
}

string Success(string_view key, string_view name, string_view info) {
    return string(key) + " " + string(name) + ": " + string(info) + "\n";
}

const string FormatStopInfo(TransportCatalogue& transport_catalogue, const Stop* const stop) {
    unordered_set<const Bus*> buses_set = transport_catalogue.GetStopBuses(stop);
    if (buses_set.empty()) {
        return "no buses";
    }

    vector<const Bus*> buses(buses_set.begin(), buses_set.end());
    string buses_string = "buses ";
    sort(buses.begin(), buses.end(), [](const Bus* bus1, const Bus* bus2) {
        return bus1->id < bus2->id;
    });
    for (const Bus* bus : buses) {
        buses_string += string(bus->id) + " ";
    }
    return buses_string;
}

const string GetStopInfo(TransportCatalogue& transport_catalogue, string_view stop_name) {
    const Stop* stop = transport_catalogue.FindStop(stop_name);
    return (
        stop == nullptr 
        ? 
        NotFound(Stop().key, stop_name)
        :
        Success(Stop().key, stop_name, FormatStopInfo(transport_catalogue, stop))
    );
}

const string FormatRouteInfo(TransportCatalogue& transport_catalogue, const Bus* bus) {
    const Route* route = transport_catalogue.GetRoute(bus->id);
    return (
        to_string(route->stops_on_route.value()) + " stops on route, " + 
        to_string(route->unique_stops.value()) + " unique stops, " + 
        to_string(route->route_distance.value()) + " route length, " +
        to_string(route->curvature.value()) + " curvature"
    );
}


const string GetBusInfo(TransportCatalogue& transport_catalogue, string_view bus_id) {
    const Bus* bus = transport_catalogue.FindBus(bus_id);

    return (
        bus == nullptr 
        ? 
        NotFound(Bus().key, bus_id)
        :
        Success(Bus().key, bus_id, FormatRouteInfo(transport_catalogue, bus))
    );
}

void ParseAndPrintStat(TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto space_pos = request.find(' ');
    std::string cmd = std::string(request.substr(0, space_pos));
    const std::string params = std::string(request.substr(space_pos + 1));

    if (cmd == Bus().key) {
        output << GetBusInfo(transport_catalogue, params);
    } else if (cmd == Stop().key) {
        output << GetStopInfo(transport_catalogue, params);
    }
}