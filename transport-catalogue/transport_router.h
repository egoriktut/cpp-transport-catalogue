#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

class TransportRouter {
public:
    TransportRouter(domain::RoutingSettings settings, const TransportCatalogue& catalogue);

    void SetSettings(domain::RoutingSettings settings);

    void BuildGraph();

    std::optional<domain::RouteResult> GetRouteResult(const std::string& from,
                                                      const std::string& to) const;

private:
    struct EdgeInfo {
        enum class Kind { Wait, Bus };
        Kind kind;
        std::string bus;
        int span_count = 0;
        std::string stop_name;
    };

    void BuildStopVertices();
    void BuildBusEdges();

    domain::RoutingSettings settings_;
    const TransportCatalogue& catalogue_;

    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    std::unordered_map<std::string, std::pair<graph::VertexId, graph::VertexId>> stop_vertices_;
    std::vector<std::string> ordered_stops_;

    std::vector<EdgeInfo> edge_info_;
};

}  // namespace transport_router
