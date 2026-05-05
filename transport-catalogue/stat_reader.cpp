#include "stat_reader.h"
#include "transport_catalogue.h"
#include <algorithm>
#include <vector>

using namespace std;

string NotFound(string_view key, string_view name) {
    return string(key) + " " + string(name) + ": not found\n";
}

string Success(string_view key, string_view name, string_view info) {
    return string(key) + " " + string(name) + ": " + string(info) + "\n";
}

const string FormatStopInfo(Stop* stop) {
    if (stop->buses.empty()) {
        return "no buses";
    }
    string buses = "buses ";
    vector<Bus*> sorted_buses(stop->buses.begin(), stop->buses.end());
    sort(sorted_buses.begin(), sorted_buses.end(), [](Bus* bus1, Bus* bus2) {
        return bus1->id < bus2->id;
    });
    for (const Bus* bus : sorted_buses) {
        buses += string(bus->id) + " ";
    }
    return buses;
}

const string GetStopInfo(TransportCatalogue& tansport_catalogue, string_view stop_name) {
    Stop* stop = tansport_catalogue.FindStop(stop_name);

    return (
        stop == nullptr 
        ? 
        NotFound(Stop().key, stop_name)
        :
        Success(Stop().key, stop_name, FormatStopInfo(stop))
    );
}

const string FormatRouteInfo(Bus* bus) {
    unordered_set<Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    
    return (
        to_string(bus->stops.size()) + " stops on route, " + 
        to_string(unique_stops.size()) + " unique stops, " + 
        to_string(bus->GetDistance()) + " route length"
    );
}


const string GetBusInfo(TransportCatalogue& tansport_catalogue, string_view bus_id) {
    Bus* bus = tansport_catalogue.FindBus(bus_id);

    return (
        bus == nullptr 
        ? 
        NotFound(Bus().key, bus_id)
        :
        Success(Bus().key, bus_id, FormatRouteInfo(bus))
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