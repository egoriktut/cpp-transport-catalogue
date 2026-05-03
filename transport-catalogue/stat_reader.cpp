#include "stat_reader.h"
#include "transport_catalogue.h"
#include <string_view>
#include <string>

void ParseAndPrintStat(TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto space_pos = request.find(' ');
    std::string cmd = std::string(request.substr(0, space_pos));
    const std::string params = std::string(request.substr(space_pos + 1));

    if (cmd == BUS_KEY) {
        output << tansport_catalogue.GetBusInfo(params);
    } else if (cmd == STOP_KEY) {
        output << tansport_catalogue.GetStopInfo(params);
    }
}