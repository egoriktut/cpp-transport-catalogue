#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;
    JsonReader reader(cin);

    reader.FillCatalogue(catalogue);

    transport_router::TransportRouter router(reader.GetRoutingSettings());
    router.BuildGraph(catalogue);

    renderer::MapRenderer renderer(reader.ParseRenderSettings());
    RequestHandler handler(catalogue, renderer, router);

    reader.ProcessRequests(handler, cout);

    return 0;
}
