#include <fstream>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace std::literals;

int main() {
    /*
    * Примерная структура программы:
    *
    * Считать JSON из stdin
    * Построить на его основе JSON базу данных транспортного справочника
    * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
    * с ответами Вывести в stdout ответы в виде JSON
    */
    
    setlocale(LC_ALL, "Russian");
    
    catalogue::TransportCatalogue db;
    renderer::MapRenderer renderer;
    json_reader::JsonReader input(std::cin, &db, &renderer);
    input.PrintResult(std::cout);
}