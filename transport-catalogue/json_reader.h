#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

class JsonReader {
public:
    explicit JsonReader(std::istream& input);

    void FillCatalogue(TransportCatalogue& catalogue) const;

    renderer::RenderSettings ParseRenderSettings() const;

    void ProcessRequests(const RequestHandler& handler, std::ostream& output) const;

private:
    json::Document document_;
    const json::Array& base_requests_;
    const json::Dict& render_settings_;
    const json::Array& stat_requests_;

    void ParseStop(TransportCatalogue& catalogue, const json::Dict& request) const;
    void ParseBus(TransportCatalogue& catalogue, const json::Dict& request) const;
    void ParseDistances(TransportCatalogue& catalogue, const json::Dict& request) const;

    json::Node MakeBusResponse(const RequestHandler& handler, const json::Dict& request) const;
    json::Node MakeStopResponse(const RequestHandler& handler, const json::Dict& request) const;
    json::Node MakeMapResponse(const RequestHandler& handler, const json::Dict& request) const;
};