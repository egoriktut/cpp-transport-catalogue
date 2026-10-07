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

/*
#include "json_builder.h"
#include <iostream>


using namespace std;

int main() {
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        cout
    );
    cout << endl;
}
*/