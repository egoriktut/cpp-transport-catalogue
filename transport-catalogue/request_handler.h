#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>

#include "domain.h"
#include "transport_catalogue.h"

class RequestHandler {
public:
    explicit RequestHandler(const TransportCatalogue& db);

    std::optional<domain::BusStat> GetBusStat(std::string_view bus_name) const;

    const std::unordered_set<const domain::Bus*>* GetBusesByStop(std::string_view stop_name) const;

private:
    const TransportCatalogue& db_;
};