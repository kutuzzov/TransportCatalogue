#include "serialization.h"

using namespace std;

void Serialize(const transport::Catalogue& tcat,
    const renderer::MapRenderer& renderer, const transport::Router& router,
    std::ostream& output) {
    serialize::TransportCatalogue database;
    for (const auto& [name, s] : tcat.GetSortedAllStops()) {
        *database.add_stop() = Serialize(s);
    }
    for (const auto& [name, b] : tcat.GetSortedAllBuses()) {
        *database.add_bus() = Serialize(b);
    }
    *database.mutable_render_settings() = GetRenderSettingSerialize(renderer.GetRenderSettings());
    *database.mutable_router() = Serialize(router);
    database.SerializeToOstream(&output);
}

serialize::Stop Serialize(const transport::Stop* stop) {
    serialize::Stop result;
    result.set_name(stop->name);
    result.add_coordinate(stop->coordinates.lat);
    result.add_coordinate(stop->coordinates.lng);
    for (const auto& [n, d] : stop->stop_distances) {
        result.add_near_stop(static_cast<string>(n));
        result.add_distance(d);
    }
    return result;
}

serialize::Bus Serialize(const transport::Bus* bus) {
    serialize::Bus result;
    result.set_name(bus->name);
    for (const auto& s : bus->stops) {
        result.add_stop(s->name);
    }
    result.set_is_circle(bus->is_circle);
    if (bus->final_stop)
        result.set_final_stop(bus->final_stop->name);
    return result;
}

serialize::Point GetPointSerialize(const json::Array& p) {
    serialize::Point result;
    result.set_x(p[0].AsDouble());
    result.set_y(p[1].AsDouble());
    return result;
}

serialize::Color GetColorSerialize(const json::Node& node) {
    serialize::Color result;
    if (node.IsArray()) {
        const json::Array& arr = node.AsArray();
        if (arr.size() == 3) {
            serialize::RGB rgb;
            rgb.set_red(arr[0].AsInt());
            rgb.set_green(arr[1].AsInt());
            rgb.set_blue(arr[2].AsInt());
            *result.mutable_rgb() = rgb;
        }
        else if (arr.size() == 4) {
            serialize::RGBA rgba;
            rgba.set_red(arr[0].AsInt());
            rgba.set_green(arr[1].AsInt());
            rgba.set_blue(arr[2].AsInt());
            rgba.set_opacity(arr[3].AsDouble());
            *result.mutable_rgba() = rgba;
        }
    }
    else if (node.IsString()) {
        result.set_name(node.AsString());
    }
    return result;
}

serialize::RenderSettings GetRenderSettingSerialize(const json::Node& render_settings) {
    const json::Dict& rs_map = render_settings.AsDict();
    serialize::RenderSettings result;
    result.set_width(rs_map.at("width"s).AsDouble());
    result.set_height(rs_map.at("height"s).AsDouble());
    result.set_padding(rs_map.at("padding"s).AsDouble());
    result.set_stop_radius(rs_map.at("stop_radius"s).AsDouble());
    result.set_line_width(rs_map.at("line_width"s).AsDouble());
    result.set_bus_label_font_size(rs_map.at("bus_label_font_size"s).AsInt());
    *result.mutable_bus_label_offset() = GetPointSerialize(rs_map.at("bus_label_offset"s).AsArray());
    result.set_stop_label_font_size(rs_map.at("stop_label_font_size"s).AsInt());
    *result.mutable_stop_label_offset() = GetPointSerialize(rs_map.at("stop_label_offset"s).AsArray());
    *result.mutable_underlayer_color() = GetColorSerialize(rs_map.at("underlayer_color"s));
    result.set_underlayer_width(rs_map.at("underlayer_width"s).AsDouble());
    for (const auto& c : rs_map.at("color_palette"s).AsArray()) {
        *result.add_color_palette() = GetColorSerialize(c);
    }
    return result;
}

serialize::RouterSettings GetRouterSettingSerialize(const json::Node& router_settings) {
    const json::Dict& rs_map = router_settings.AsDict();
    serialize::RouterSettings result;
    result.set_bus_wait_time(rs_map.at("bus_wait_time"s).AsInt());
    result.set_bus_velocity(rs_map.at("bus_velocity"s).AsDouble());
    return result;
}

serialize::Graph GetGraphSerialize(const graph::DirectedWeightedGraph<double>& g) {
    serialize::Graph result;
    size_t vertex_count = g.GetVertexCount();
    size_t edge_count = g.GetEdgeCount();
    for (size_t i = 0; i < edge_count; ++i) {
        const graph::Edge<double>& edge = g.GetEdge(i);
        serialize::Edge s_edge;
        s_edge.set_name(edge.name);
        s_edge.set_quality(edge.quality);
        s_edge.set_from(edge.from);
        s_edge.set_to(edge.to);
        s_edge.set_weight(edge.weight);
        *result.add_edge() = s_edge;
    }
    for (size_t i = 0; i < vertex_count; ++i) {
        serialize::Vertex vertex;
        for (const auto& edge_id : g.GetIncidentEdges(i)) {
            vertex.add_edge_id(edge_id);
        }
        *result.add_vertex() = vertex;
    }
    return result;
}

serialize::Router Serialize(const transport::Router& router) {
    serialize::Router result;
    *result.mutable_router_settings() = GetRouterSettingSerialize(router.GetSettings());
    *result.mutable_graph() = GetGraphSerialize(router.GetGraph());
    for (const auto& [n, id] : router.GetStopIds()) {
        serialize::StopId si;
        si.set_name(n);
        si.set_id(id);
        *result.add_stop_id() = si;
    }
    return result;
}

void SetStopsDistances(transport::Catalogue& tcat, const serialize::TransportCatalogue& database) {
    for (size_t i = 0; i < database.stop_size(); ++i) {
        const serialize::Stop& stop_i = database.stop(i);
        transport::Stop* from = tcat.FindStop(stop_i.name());
        for (size_t j = 0; j < stop_i.near_stop_size(); ++j) {
            tcat.SetDistance(from, tcat.FindStop(stop_i.near_stop(j)), stop_i.distance(j));
        }
    }
}

void AddStopFromDB(transport::Catalogue& tcat, const serialize::TransportCatalogue& database) {
    for (size_t i = 0; i < database.stop_size(); ++i) {
        const serialize::Stop& stop_i = database.stop(i);
        tcat.AddStop(stop_i.name(), { stop_i.coordinate(0), stop_i.coordinate(1) });
    }
    SetStopsDistances(tcat, database);
}

void AddBusFromDB(transport::Catalogue& tcat, const serialize::TransportCatalogue& database) {
    for (size_t i = 0; i < database.bus_size(); ++i) {
        const serialize::Bus& bus_i = database.bus(i);
        std::vector<transport::Stop*> stops(bus_i.stop_size());
        for (size_t j = 0; j < stops.size(); ++j) {
            stops[j] = tcat.FindStop(bus_i.stop(j));
        }
        tcat.AddBus(bus_i.name(), stops, bus_i.is_circle());
        if (!bus_i.final_stop().empty()) {
            transport::Bus* bus = tcat.FindBus(bus_i.name());
            bus->final_stop = tcat.FindStop(bus_i.final_stop());
        }
    }
}

json::Node ToNode(const serialize::Point& p) {
    return json::Node(json::Array{ {p.x()}, {p.y()} });
}

json::Node ToNode(const serialize::Color& c) {
    if (!c.name().empty()) {
        return json::Node(c.name());
    }
    else if (c.has_rgb()) {
        const serialize::RGB& rgb = c.rgb();
        return json::Node(json::Array{ {rgb.red()}, {rgb.green()}, {rgb.blue()} });
    }
    else if (c.has_rgba()) {
        const serialize::RGBA& rgba = c.rgba();
        return json::Node(json::Array{ {rgba.red()}, {rgba.green()}, {rgba.blue()}, {rgba.opacity()} });
    }
    else
        return json::Node("none"s);
}

json::Node ToNode(const google::protobuf::RepeatedPtrField<serialize::Color>& cv) {
    json::Array result;
    result.reserve(cv.size());
    for (const auto& c : cv) {
        result.emplace_back(ToNode(c));
    }
    return json::Node(std::move(result));
}

json::Node GetRenderSettingsFromDB(const serialize::TransportCatalogue& database) {
    const serialize::RenderSettings& rs = database.render_settings();
    return json::Node(json::Dict{
                    {{"width"s},{ rs.width() }},
                    {{"height"s},{ rs.height() }},
                    {{"padding"s},{ rs.padding() }},
                    {{"stop_radius"s},{ rs.stop_radius() }},
                    {{"line_width"s},{ rs.line_width() }},
                    {{"bus_label_font_size"s},{ rs.bus_label_font_size() }},
                    {{"bus_label_offset"s},ToNode(rs.bus_label_offset())},
                    {{"stop_label_font_size"s},{rs.stop_label_font_size()}},
                    {{"stop_label_offset"s},ToNode(rs.stop_label_offset())},
                    {{"underlayer_color"s},ToNode(rs.underlayer_color())},
                    {{"underlayer_width"s},{rs.underlayer_width()}},
                    {{"color_palette"s},ToNode(rs.color_palette())},
        });
}

json::Node GetRouterSettingsFromDB(const serialize::Router& router) {
    const serialize::RouterSettings& rs = router.router_settings();
    return json::Node(json::Dict{
                    {{"bus_wait_time"s},{ rs.bus_wait_time() }},
                    {{"bus_velocity"s},{ rs.bus_velocity() }}
        });
}

graph::DirectedWeightedGraph<double> GetGraphFromDB(const serialize::Router& router) {
    const serialize::Graph& g = router.graph();
    std::vector<graph::Edge<double>> edges(g.edge_size());
    std::vector<std::vector<graph::EdgeId>> incidence_lists(g.vertex_size());
    for (size_t i = 0; i < edges.size(); ++i) {
        const serialize::Edge& e = g.edge(i);
        edges[i] = { e.name(), static_cast<size_t>(e.quality()),
        static_cast<size_t>(e.from()), static_cast<size_t>(e.to()), e.weight() };
    }
    for (size_t i = 0; i < incidence_lists.size(); ++i) {
        const serialize::Vertex& v = g.vertex(i);
        incidence_lists[i].reserve(v.edge_id_size());
        for (const auto& id : v.edge_id()) {
            incidence_lists[i].push_back(id);
        }
    }
    return graph::DirectedWeightedGraph<double>(edges, incidence_lists);
}

std::map<std::string, graph::VertexId> GetStopIdsFromDB(const serialize::Router& router) {
    std::map<std::string, graph::VertexId> result;
    for (const auto& s : router.stop_id()) {
        result[s.name()] = s.id();
    }
    return result;
}

std::tuple<transport::Catalogue, renderer::MapRenderer, transport::Router,
    graph::DirectedWeightedGraph<double>, std::map<std::string, graph::VertexId>>
    Deserialize(std::istream& input) {
    serialize::TransportCatalogue database;
    database.ParseFromIstream(&input);
    transport::Catalogue tcat;
    renderer::MapRenderer renderer(GetRenderSettingsFromDB(database));
    transport::Router router(GetRouterSettingsFromDB(database.router()));
    AddStopFromDB(tcat, database);
    AddBusFromDB(tcat, database);
    return { std::move(tcat), std::move(renderer), std::move(router),
                            GetGraphFromDB(database.router()),
                            GetStopIdsFromDB(database.router()) };
}