#pragma once
#include <cmath>
#include <cstddef>
#include <deque>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <set>

#include "domain.h"
#include "geo.h"


class TransportCatalogue {
private:
    std::deque<domain::Stop> stops_storage_;
    std::deque<domain::Bus> bus_storage_;
    std::unordered_map<std::string_view, const domain::Stop*> stops_;
    std::unordered_map<std::string_view, const domain::Bus*> buses_;

    std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, double, domain::StopPairHash> distances_;
    std::unordered_map<std::string_view, std::unordered_set<const domain::Bus*>> stop_buses_;

private:
    void ComputeRouteDistance(const domain::Bus* bus, domain::Route& route) const;
    void ComputeUniqueStops(const domain::Bus* bus, domain::Route& route) const;

public:
    TransportCatalogue();
    ~TransportCatalogue();

    void SetDistanceToStop(std::string_view name, const std::vector<std::pair<std::string, unsigned>>& distance);
    void SetDistance(std::string_view from, std::string_view to, double distance);
    std::optional<double> GetDistance(std::string_view from, std::string_view to) const;

    void AddStop(std::string_view name, geo::Coordinates coordinates);
    const domain::Stop* FindStop(std::string_view name) const;
    const std::unordered_set<const domain::Bus*>& GetStopBuses(const domain::Stop* stop) const;

    void AddBus(std::string_view id,
                const std::vector<std::string_view>& route,
                const std::vector<std::string_view>& endpoints,
                bool is_roundtrip);    const domain::Bus* FindBus(std::string_view id) const;
    const domain::Route GetRoute(std::string_view id) const;

    const std::unordered_map<std::string_view, const domain::Bus*>& GetAllBuses() const {
        return buses_;
    }
};