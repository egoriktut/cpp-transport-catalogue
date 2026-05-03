#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

void ParseAndPrintStat(TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);