#include "json_reader.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace json;

namespace {

svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    const auto& arr = node.AsArray();
    if (arr.size() == 3) {
        return svg::Rgb{
            static_cast<uint8_t>(arr[0].AsInt()),
            static_cast<uint8_t>(arr[1].AsInt()),
            static_cast<uint8_t>(arr[2].AsInt())
        };
    }
    return svg::Rgba{
        static_cast<uint8_t>(arr[0].AsInt()),
        static_cast<uint8_t>(arr[1].AsInt()),
        static_cast<uint8_t>(arr[2].AsInt()),
        arr[3].AsDouble()
    };
}

svg::Point ParsePoint(const json::Array& arr) {
    return {arr[0].AsDouble(), arr[1].AsDouble()};
}

}  // namespace

JsonReader::JsonReader(istream& input)
    : document_(Load(input))
    , base_requests_(document_.GetRoot().AsDict().at("base_requests").AsArray())
    , render_settings_(document_.GetRoot().AsDict().at("render_settings").AsDict())
    , stat_requests_(document_.GetRoot().AsDict().at("stat_requests").AsArray()) {
}

void JsonReader::ParseStop(TransportCatalogue& catalogue, const Dict& request) const {
    const string name = request.at("name").AsString();
    const double lat = request.at("latitude").AsDouble();
    const double lng = request.at("longitude").AsDouble();
    catalogue.AddStop(name, {lat, lng});
}

void JsonReader::ParseBus(TransportCatalogue& catalogue, const Dict& request) const {
    const string name = request.at("name").AsString();
    const bool is_roundtrip = request.at("is_roundtrip").AsBool();
    const auto& stops_array = request.at("stops").AsArray();

    vector<string_view> route;
    route.reserve(stops_array.size() + (is_roundtrip ? 0 : stops_array.size() - 1));

    for (const auto& stop : stops_array) {
        route.push_back(stop.AsString());
    }

    if (!is_roundtrip) {
        for (auto it = next(stops_array.rbegin()); it != stops_array.rend(); ++it) {
            route.push_back(it->AsString());
        }
    }

    vector<string_view> endpoints;
    if (is_roundtrip) {
        if (!stops_array.empty()) {
            endpoints.push_back(stops_array.front().AsString());
        }
    } else {
        if (!stops_array.empty()) {
            endpoints.push_back(stops_array.front().AsString());
            if (stops_array.size() > 1) {
                endpoints.push_back(stops_array.back().AsString());
            }
        }
    }

    catalogue.AddBus(name, route, endpoints, is_roundtrip);
}

void JsonReader::ParseDistances(TransportCatalogue& catalogue, const Dict& request) const {
    const string from = request.at("name").AsString();
    const auto& road_distances = request.at("road_distances").AsDict();

    for (const auto& [to, distance] : road_distances) {
        catalogue.SetDistance(from, to, distance.AsInt());
    }
}

void JsonReader::FillCatalogue(TransportCatalogue& catalogue) const {
    for (const auto& request : base_requests_) {
        const auto& dict = request.AsDict();
        if (dict.at("type").AsString() == "Stop") {
            ParseStop(catalogue, dict);
        }
    }

    for (const auto& request : base_requests_) {
        const auto& dict = request.AsDict();
        if (dict.at("type").AsString() == "Stop") {
            ParseDistances(catalogue, dict);
        }
    }

    for (const auto& request : base_requests_) {
        const auto& dict = request.AsDict();
        if (dict.at("type").AsString() == "Bus") {
            ParseBus(catalogue, dict);
        }
    }
}

renderer::RenderSettings JsonReader::ParseRenderSettings() const {
    renderer::RenderSettings settings;
    settings.width = render_settings_.at("width").AsDouble();
    settings.height = render_settings_.at("height").AsDouble();
    settings.padding = render_settings_.at("padding").AsDouble();

    settings.line_width = render_settings_.at("line_width").AsDouble();
    settings.stop_radius = render_settings_.at("stop_radius").AsDouble();

    settings.bus_label_font_size = render_settings_.at("bus_label_font_size").AsInt();
    settings.bus_label_offset = ParsePoint(render_settings_.at("bus_label_offset").AsArray());

    settings.stop_label_font_size = render_settings_.at("stop_label_font_size").AsInt();
    settings.stop_label_offset = ParsePoint(render_settings_.at("stop_label_offset").AsArray());

    settings.underlayer_color = ParseColor(render_settings_.at("underlayer_color"));
    settings.underlayer_width = render_settings_.at("underlayer_width").AsDouble();

    for (const auto& color : render_settings_.at("color_palette").AsArray()) {
        settings.color_palette.push_back(ParseColor(color));
    }
    return settings;
}

Node JsonReader::MakeBusResponse(const RequestHandler& handler, const Dict& request) const {
    const int id = request.at("id").AsInt();
    const string name = request.at("name").AsString();

    const auto bus_stat = handler.GetBusStat(name);
    if (!bus_stat) {
        return Dict{
            {"request_id", id},
            {"error_message", "not found"s}
        };
    }

    return Dict{
        {"curvature", bus_stat->curvature},
        {"request_id", id},
        {"route_length", bus_stat->route_length},
        {"stop_count", bus_stat->stop_count},
        {"unique_stop_count", bus_stat->unique_stop_count}
    };
}

Node JsonReader::MakeStopResponse(const RequestHandler& handler, const Dict& request) const {
    const int id = request.at("id").AsInt();
    const string name = request.at("name").AsString();

    const auto buses = handler.GetBusesByStop(name);
    if (!buses) {
        return Dict{
            {"request_id", id},
            {"error_message", "not found"s}
        };
    }

    vector<string> bus_names;
    bus_names.reserve(buses->size());
    for (const auto* bus : *buses) {
        bus_names.push_back(bus->id);
    }
    sort(bus_names.begin(), bus_names.end());

    Array buses_array;
    buses_array.reserve(bus_names.size());
    for (auto& n : bus_names) {
        buses_array.push_back(move(n));
    }

    return Dict{
        {"buses", move(buses_array)},
        {"request_id", id}
    };
}

Node JsonReader::MakeMapResponse(const renderer::MapRenderer& renderer,
                                 const TransportCatalogue& catalogue,
                                 const Dict& request) const {
    const int id = request.at("id").AsInt();

    ostringstream svg_out;
    renderer.Render(catalogue).Render(svg_out);

    return Dict{
        {"map", svg_out.str()},
        {"request_id", id}
    };
}

void JsonReader::ProcessRequests(const RequestHandler& handler,
                                 const renderer::MapRenderer& renderer,
                                 const TransportCatalogue& catalogue,
                                 ostream& output) const {
    Array responses;
    responses.reserve(stat_requests_.size());

    for (const auto& request : stat_requests_) {
        const auto& dict = request.AsDict();
        const string type = dict.at("type").AsString();

        if (type == "Bus") {
            responses.push_back(MakeBusResponse(handler, dict));
        } else if (type == "Stop") {
            responses.push_back(MakeStopResponse(handler, dict));
        } else if (type == "Map") {
            responses.push_back(MakeMapResponse(renderer, catalogue, dict));
        }
    }

    Print(Document(move(responses)), output);
}