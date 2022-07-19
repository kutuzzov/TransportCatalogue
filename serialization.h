#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

void Serialize(const transport::Catalogue& tcat,
    const renderer::MapRenderer& renderer,
    const transport::Router& router,
    std::ostream& output);

serialize::Stop Serialize(const transport::Stop* stop);

serialize::Bus Serialize(const transport::Bus* bus);

serialize::RenderSettings GetRenderSettingSerialize(const json::Node& render_settings);

serialize::RouterSettings GetRouterSettingSerialize(const json::Node& router_settings);

serialize::Router Serialize(const transport::Router& router);

std::tuple<transport::Catalogue, renderer::MapRenderer, transport::Router,
    graph::DirectedWeightedGraph<double>, std::map<std::string, graph::VertexId>> Deserialize(std::istream& input);