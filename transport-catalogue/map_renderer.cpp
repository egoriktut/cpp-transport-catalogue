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

void AddTextWithUnderlayer(
    Document& doc,
    const Point& pos,
    const Point& offset,
    int font_size,
    const string& data,
    const Color& text_color,
    const Color& underlayer_color,
    double underlayer_width,
    bool bold
) {
    Text underlayer;
    underlayer.SetPosition(pos)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily("Verdana")
        .SetData(data)
        .SetFillColor(underlayer_color)
        .SetStrokeColor(underlayer_color)
        .SetStrokeWidth(underlayer_width)
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

}  // namespace

MapRenderer::MapRenderer(RenderSettings settings)
    : settings_(move(settings)) {
}

void MapRenderer::SetSettings(RenderSettings settings) {
    settings_ = move(settings);
}

Document MapRenderer::Render(const TransportCatalogue& catalogue) const {
    Document doc;

    vector<const Bus*> buses;
    for (const auto& [name, bus_ptr] : catalogue.GetAllBuses()) {
        buses.push_back(bus_ptr);
    }
    sort(buses.begin(), buses.end(),
         [](const Bus* lhs, const Bus* rhs) { return lhs->id < rhs->id; });

    const auto used_stops = CollectSortedUsedStops(catalogue);

    vector<geo::Coordinates> coords;
    coords.reserve(used_stops.size());
    for (const Stop* stop : used_stops) {
        coords.push_back(stop->coordinates);
    }

    const SphereProjector projector(
        coords.begin(), coords.end(),
        settings_.width, settings_.height, settings_.padding);

    auto color_for = [&](size_t index) -> const Color& {
        return settings_.color_palette[index % settings_.color_palette.size()];
    };

    // Линии маршрутов
    {
        size_t color_index = 0;
        for (const Bus* bus : buses) {
            if (bus->stops.empty()) {
                continue;
            }
            const Color& color = color_for(color_index);
            ++color_index;

            Polyline line;
            line.SetFillColor(NoneColor)
                .SetStrokeColor(color)
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(StrokeLineCap::ROUND)
                .SetStrokeLineJoin(StrokeLineJoin::ROUND);

            for (const Stop* stop : bus->stops) {
                line.AddPoint(projector(stop->coordinates));
            }
            doc.Add(line);
        }
    }

    // Названия маршрутов
    {
        size_t color_index = 0;
        for (const Bus* bus : buses) {
            if (bus->stops.empty()) {
                continue;
            }
            const Color& color = color_for(color_index);
            ++color_index;

            for (size_t i = 0; i < bus->endpoints.size(); ++i) {
                const Stop* stop = bus->endpoints[i];
                if (i == 1 && stop == bus->endpoints[0]) {
                    continue;
                }
                const Point pos = projector(stop->coordinates);
                AddTextWithUnderlayer(
                    doc, pos, settings_.bus_label_offset,
                    settings_.bus_label_font_size, bus->id, color,
                    settings_.underlayer_color, settings_.underlayer_width,
                    /*bold=*/true
                );
            }
        }
    }

    // Круги остановок
    for (const Stop* stop : used_stops) {
        Circle circle;
        circle.SetCenter(projector(stop->coordinates))
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white");
        doc.Add(circle);
    }

    // Названия остановок
    for (const Stop* stop : used_stops) {
        const Point pos = projector(stop->coordinates);
        AddTextWithUnderlayer(
            doc, pos, settings_.stop_label_offset,
            settings_.stop_label_font_size, stop->name, Color{"black"},
            settings_.underlayer_color, settings_.underlayer_width,
            /*bold=*/false
        );
    }

    return doc;
}

}  // namespace renderer