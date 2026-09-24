#include "map_renderer.h"

#include <algorithm>
#include <set>

using namespace std;
using namespace svg;
using namespace domain;

namespace renderer {

namespace {

vector<const Stop*> CollectSortedUsedStops(const TransportCatalogue& catalogue) {
    set<string_view> used;
    for (const auto& [bus_name, bus_ptr] : catalogue.GetAllBuses()) {
        for (const Stop* stop : bus_ptr->stops) {
            used.insert(stop->name);
        }
    }
    vector<const Stop*> stops;
    stops.reserve(used.size());
    for (string_view name : used) {
        const Stop* stop = catalogue.FindStop(name);
        if (stop != nullptr) {
            stops.push_back(stop);
        }
    }
    return stops;
}

vector<const Bus*> CollectSortedBuses(const TransportCatalogue& catalogue) {
    vector<const Bus*> buses;
    for (const auto& [name, bus_ptr] : catalogue.GetAllBuses()) {
        buses.push_back(bus_ptr);
    }
    sort(buses.begin(), buses.end(),
         [](const Bus* lhs, const Bus* rhs) { return lhs->id < rhs->id; });
    return buses;
}

}  // namespace

MapRenderer::MapRenderer(RenderSettings settings)
    : settings_(move(settings)) {
}

void MapRenderer::SetSettings(RenderSettings settings) {
    settings_ = move(settings);
}

const Color& MapRenderer::ColorFor(size_t index) const {
    return settings_.color_palette[index % settings_.color_palette.size()];
}

void MapRenderer::AddTextWithUnderlayer(
    Document& doc,
    const Point& pos,
    const Point& offset,
    int font_size,
    const string& data,
    const Color& text_color,
    bool bold
) const {
    Text underlayer;
    underlayer.SetPosition(pos)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily("Verdana")
        .SetData(data)
        .SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    if (bold) {
        underlayer.SetFontWeight("bold");
    }
    doc.Add(underlayer);

    Text label;
    label.SetPosition(pos)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily("Verdana")
        .SetData(data)
        .SetFillColor(text_color);
    if (bold) {
        label.SetFontWeight("bold");
    }
    doc.Add(label);
}

void MapRenderer::RenderRouteLines(const RenderContext& ctx) const {
    size_t color_index = 0;
    for (const Bus* bus : ctx.buses) {
        if (bus->stops.empty()) {
            continue;
        }
        const Color& color = ColorFor(color_index);
        ++color_index;

        Polyline line;
        line.SetFillColor(NoneColor)
            .SetStrokeColor(color)
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);

        for (const Stop* stop : bus->stops) {
            line.AddPoint(ctx.projector(stop->coordinates));
        }
        ctx.doc.Add(line);
    }
}

void MapRenderer::RenderRouteNames(const RenderContext& ctx) const {
    size_t color_index = 0;
    for (const Bus* bus : ctx.buses) {
        if (bus->stops.empty()) {
            continue;
        }
        const Color& color = ColorFor(color_index);
        ++color_index;

        for (size_t i = 0; i < bus->endpoints.size(); ++i) {
            const Stop* stop = bus->endpoints[i];
            if (i == 1 && stop == bus->endpoints[0]) {
                continue;
            }
            AddTextWithUnderlayer(
                ctx.doc, ctx.projector(stop->coordinates),
                settings_.bus_label_offset,
                settings_.bus_label_font_size, bus->id, color,
                /*bold=*/true
            );
        }
    }
}

void MapRenderer::RenderStopCircles(const RenderContext& ctx) const {
    for (const Stop* stop : ctx.used_stops) {
        Circle circle;
        circle.SetCenter(ctx.projector(stop->coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white");
        ctx.doc.Add(circle);
    }
}

void MapRenderer::RenderStopNames(const RenderContext& ctx) const {
    for (const Stop* stop : ctx.used_stops) {
        AddTextWithUnderlayer(
            ctx.doc, ctx.projector(stop->coordinates),
            settings_.stop_label_offset,
            settings_.stop_label_font_size, stop->name, Color{"black"},
            /*bold=*/false
        );
    }
}

Document MapRenderer::Render(const TransportCatalogue& catalogue) const {
    Document doc;

    const auto buses = CollectSortedBuses(catalogue);
    const auto used_stops = CollectSortedUsedStops(catalogue);

    vector<geo::Coordinates> coords;
    coords.reserve(used_stops.size());
    for (const Stop* stop : used_stops) {
        coords.push_back(stop->coordinates);
    }

    const SphereProjector projector(
        coords.begin(), coords.end(),
        settings_.width, settings_.height, settings_.padding);

    const RenderContext ctx{doc, projector, buses, used_stops};

    RenderRouteLines(ctx);
    RenderRouteNames(ctx);
    RenderStopCircles(ctx);
    RenderStopNames(ctx);

    return doc;
}

}  // namespace renderer