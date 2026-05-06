#pragma once

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

std::string NotFound(std::string_view key, std::string_view name);
std::string Success(std::string_view key, std::string_view name, std::string_view info);

const std::string FormatStopInfo(TransportCatalogue& tansport_catalogue, const Stop* stop);
const std::string GetStopInfo(TransportCatalogue& tansport_catalogue, std::string_view stop_name);

const std::string FormatRouteInfo(TransportCatalogue& tansport_catalogue, const Bus* bus);
const std::string GetBusInfo(TransportCatalogue& tansport_catalogue, std::string_view bus_id);

void ParseAndPrintStat(TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);