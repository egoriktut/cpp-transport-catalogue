#include "transport_router.h"

#include "geo.h"

namespace transport_router {

TransportRouter::TransportRouter(domain::RoutingSettings settings)
    : settings_(settings) {
}

void TransportRouter::SetSettings(domain::RoutingSettings settings) {
    settings_ = settings;
}

void TransportRouter::BuildGraph(const TransportCatalogue& catalogue) {
    edge_info_.clear();
    stop_vertices_.clear();

    const auto& all_stops = catalogue.GetAllStops();

    const size_t vertex_count = all_stops.size() * 2;
    graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(vertex_count);

    size_t k = 0;
    for (const auto& [name_sv, stop_ptr] : all_stops) {
        stop_vertices_[std::string(name_sv)] = {2 * k, 2 * k + 1};
        ++k;
    }

    const double wait_w = static_cast<double>(settings_.bus_wait_time);
    for (const auto& [name, vp] : stop_vertices_) {
        graph_->AddEdge({vp.first, vp.second, wait_w});
        edge_info_.push_back({EdgeInfo::Kind::Wait, "", 0, name});
    }

    const double velocity_m_per_min = settings_.bus_velocity * 1000.0 / 60.0;
    for (const auto& [bus_name_sv, bus_ptr] : catalogue.GetAllBuses()) {
        const auto& stops = bus_ptr->stops;
        const std::string bus_name(bus_name_sv);

        for (size_t i = 0; i + 1 < stops.size(); ++i) {
            double dist_accum = 0.0;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                if (stops[i] == stops[j]) {
                    continue;
                }

                double d;
                if (auto opt = catalogue.GetDistance(stops[j-1]->name, stops[j]->name)) {
                    d = *opt;
                } else {
                    d = geo::ComputeDistance(stops[j-1]->coordinates,
                                             stops[j]->coordinates);
                }
                dist_accum += d;

                auto [_, from_bus_v] = stop_vertices_.at(stops[i]->name);
                auto [to_wait_v, __] = stop_vertices_.at(stops[j]->name);
                double t = dist_accum / velocity_m_per_min;

                graph_->AddEdge({from_bus_v, to_wait_v, t});
                edge_info_.push_back({
                    EdgeInfo::Kind::Bus, bus_name,
                    static_cast<int>(j - i), ""
                });
            }
        }
    }

    router_ = std::make_unique<graph::Router<double>>(*graph_);
}

std::optional<domain::RouteResult>
TransportRouter::GetRouteResult(const std::string& from,
                                const std::string& to) const {
    auto it_from = stop_vertices_.find(from);
    auto it_to   = stop_vertices_.find(to);
    if (it_from == stop_vertices_.end() || it_to == stop_vertices_.end()) {
        return std::nullopt;
    }
    if (from == to) {
        return domain::RouteResult{0.0, {}};
    }

    graph::VertexId start  = it_from->second.first;
    graph::VertexId finish = it_to->second.first;

    auto route = router_->BuildRoute(start, finish);
    if (!route) return std::nullopt;

    domain::RouteResult result;
    result.total_time = route->weight;

    for (graph::EdgeId eid : route->edges) {
        const auto& info = edge_info_[eid];
        if (info.kind == EdgeInfo::Kind::Wait) {
            result.items.push_back(domain::RouteItemWait{
                info.stop_name,
                static_cast<double>(settings_.bus_wait_time)
            });
        } else {
            const auto& e = graph_->GetEdge(eid);
            result.items.push_back(domain::RouteItemBus{
                info.bus, info.span_count, e.weight
            });
        }
    }
    return result;
}

}  // namespace transport_router
