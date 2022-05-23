#include <fstream>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

int main() {
    setlocale(LC_ALL, "Russian");
    
    catalogue::TransportCatalogue db;
    renderer::MapRenderer renderer;
    transport_router::TransportRouter router;
    
    json_reader::JsonReader input(std::cin, &db, &renderer, &router);
    input.PrintResult(std::cout);
}