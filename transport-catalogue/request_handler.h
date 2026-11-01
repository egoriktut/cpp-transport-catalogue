#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db,
                   const renderer::MapRenderer& renderer,
                   const transport_router::TransportRouter& router);

    std::optional<domain::BusStat> GetBusStat(std::string_view bus_name) const;

    const std::unordered_set<const domain::Bus*>* GetBusesByStop(std::string_view stop_name) const;

    svg::Document RenderMap() const;

    std::optional<domain::RouteResult> GetRoute(
        std::string_view from, std::string_view to) const;

private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
    const transport_router::TransportRouter& router_;
};