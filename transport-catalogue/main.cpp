#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;
    JsonReader reader(cin);

    reader.FillCatalogue(catalogue);

    renderer::MapRenderer renderer(reader.ParseRenderSettings());
    RequestHandler handler(catalogue, renderer);

    reader.ProcessRequests(handler, cout);

    return 0;
}