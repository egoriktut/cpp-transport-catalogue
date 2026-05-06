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

const string FormatStopInfo(TransportCatalogue& tansport_catalogue, const Stop* const stop) {
    unordered_set<const Bus*> buses_set = tansport_catalogue.GetStopBuses(stop);
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

const string GetStopInfo(TransportCatalogue& tansport_catalogue, string_view stop_name) {
    const Stop* stop = tansport_catalogue.FindStop(stop_name);
    return (
        stop == nullptr 
        ? 
        NotFound(Stop().key, stop_name)
        :
        Success(Stop().key, stop_name, FormatStopInfo(tansport_catalogue, stop))
    );
}

const string FormatRouteInfo(TransportCatalogue& tansport_catalogue, const Bus* bus) {
    const BusView bus_view = tansport_catalogue.GetBusInfo(bus);
    return (
        to_string(bus_view.stops_on_route) + " stops on route, " + 
        to_string(bus_view.unique_stops) + " unique stops, " + 
        to_string(bus_view.route_distance) + " route length"
    );
}


const string GetBusInfo(TransportCatalogue& tansport_catalogue, string_view bus_id) {
    const Bus* bus = tansport_catalogue.FindBus(bus_id);

    return (
        bus == nullptr 
        ? 
        NotFound(Bus().key, bus_id)
        :
        Success(Bus().key, bus_id, FormatRouteInfo(tansport_catalogue, bus))
    );
}

void ParseAndPrintStat(TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto space_pos = request.find(' ');
    std::string cmd = std::string(request.substr(0, space_pos));
    const std::string params = std::string(request.substr(space_pos + 1));

    if (cmd == Bus().key) {
        output << GetBusInfo(tansport_catalogue, params);
    } else if (cmd == Stop().key) {
        output << GetStopInfo(tansport_catalogue, params);
    }
}