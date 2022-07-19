#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"

#include <utility>
#include <string>
#include <string_view>

class RequestHandler {
public:
    RequestHandler(const transport::Catalogue& catalogue,
        const transport::Router& router, const renderer::MapRenderer& renderer);

    void JsonStatRequests(const json::Node& json_doc, std::ostream& output);

    svg::Document RenderMap() const;

private:
    const transport::Catalogue& db_;
    const transport::Router& router_;
    const renderer::MapRenderer& renderer_;

    json::Node FindStopRequestProcessing(const json::Dict& request_map);
    json::Node FindBusRequestProcessing(const json::Dict& request_map);
    json::Node BuildMapRequestProcessing(const json::Dict& request_map);
    json::Node BuildRouteRequestProcessing(const json::Dict& request_map);
};