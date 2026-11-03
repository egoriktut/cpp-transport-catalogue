#include "transport_router.h"

#include "geo.h"

namespace transport_router {

namespace {

constexpr double KILOMETRE_IN_METRES = 1000.0;
constexpr double MINUTES_IN_HOUR = 60.0;

}  // namespace

TransportRouter::TransportRouter(domain::RoutingSettings settings, const TransportCatalogue& catalogue)
    : settings_(settings)
    , catalogue_(catalogue) {
}

void TransportRouter::SetSettings(domain::RoutingSettings settings) {
    settings_ = settings;
}

void TransportRouter::BuildGraph() {
    edge_info_.clear();
    stop_vertices_.clear();
    ordered_stops_.clear();
    router_.reset();

    graph_ = graph::DirectedWeightedGraph<double>{catalogue_.GetAllStops().size() * 2};

    BuildStopVertices();
    BuildBusEdges();

    router_ = std::make_unique<graph::Router<double>>(graph_);
}

void TransportRouter::BuildStopVertices() {
    for (const auto& stop : catalogue_.GetStopsInInsertionOrder()) {
        if (catalogue_.FindStop(stop.name) != &stop) {
            continue;
        }
        const graph::VertexId wait_vertex = 2 * static_cast<graph::VertexId>(ordered_stops_.size());
        stop_vertices_.emplace(stop.name, std::make_pair(wait_vertex, wait_vertex + 1));
        ordered_stops_.push_back(stop.name);
    }

    const double wait_edge_weight = static_cast<double>(settings_.bus_wait_time);
    for (const auto& stop_name : ordered_stops_) {
        const auto [wait_vertex, bus_vertex] = stop_vertices_.at(stop_name);
        graph_.AddEdge({wait_vertex, bus_vertex, wait_edge_weight});
        edge_info_.push_back({EdgeInfo::Kind::Wait, "", 0, stop_name});
    }
}

void TransportRouter::BuildBusEdges() {
    const double bus_velocity_metres_per_minute =
        settings_.bus_velocity * KILOMETRE_IN_METRES / MINUTES_IN_HOUR;

    for (const auto& bus : catalogue_.GetBusesInInsertionOrder()) {
        const domain::Bus* bus_ptr = catalogue_.FindBus(bus.id);
        if (bus_ptr != &bus) {
            continue;
        }

        const auto& route_stops = bus_ptr->stops;
        const std::string bus_name = bus_ptr->id;

        for (size_t boarding_index = 0; boarding_index + 1 < route_stops.size(); ++boarding_index) {
            double travelled_distance = 0.0;

            for (size_t alighting_index = boarding_index + 1; alighting_index < route_stops.size();
                 ++alighting_index) {
                const domain::Stop* boarding_stop = route_stops[boarding_index];
                const domain::Stop* alighting_stop = route_stops[alighting_index];
                const domain::Stop* previous_stop = route_stops[alighting_index - 1];

                const auto road_distance =
                    catalogue_.GetDistance(previous_stop->name, alighting_stop->name);
                travelled_distance += road_distance.value_or(
                    geo::ComputeDistance(previous_stop->coordinates, alighting_stop->coordinates));

                if (boarding_stop == alighting_stop) {
                    continue;
                }

                const graph::VertexId boarding_bus_vertex =
                    stop_vertices_.at(boarding_stop->name).second;
                const graph::VertexId alighting_wait_vertex =
                    stop_vertices_.at(alighting_stop->name).first;
                const double travel_time = travelled_distance / bus_velocity_metres_per_minute;
                const int span_count = static_cast<int>(alighting_index - boarding_index);

                graph_.AddEdge({boarding_bus_vertex, alighting_wait_vertex, travel_time});
                edge_info_.push_back({EdgeInfo::Kind::Bus, bus_name, span_count, ""});
            }
        }
    }
}

std::optional<domain::RouteResult> TransportRouter::GetRouteResult(const std::string& from,
                                                                   const std::string& to) const {
    const auto origin = stop_vertices_.find(from);
    const auto destination = stop_vertices_.find(to);
    if (origin == stop_vertices_.end() || destination == stop_vertices_.end()) {
        return std::nullopt;
    }
    if (from == to) {
        return domain::RouteResult{0.0, {}};
    }

    const graph::VertexId start_vertex = origin->second.first;
    const graph::VertexId finish_vertex = destination->second.first;

    const auto route = router_->BuildRoute(start_vertex, finish_vertex);
    if (!route) {
        return std::nullopt;
    }

    domain::RouteResult result;
    result.total_time = route->weight;

    for (const graph::EdgeId edge_id : route->edges) {
        const auto& info = edge_info_[edge_id];
        if (info.kind == EdgeInfo::Kind::Wait) {
            result.items.push_back(domain::RouteItemWait{
                info.stop_name,
                static_cast<double>(settings_.bus_wait_time)
            });
        } else {
            result.items.push_back(domain::RouteItemBus{
                info.bus,
                info.span_count,
                graph_.GetEdge(edge_id).weight
            });
        }
    }

    return result;
}

}  // namespace transport_router
