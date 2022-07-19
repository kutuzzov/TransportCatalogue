#include "transport_router.h"

#include <string>
#include <string_view>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;

namespace transport {

    Router::Router(const json::Node& settings_node) {
        if (settings_node.IsNull()) return;
        SetSettings(settings_node);
    }

    Router::Router(const json::Node& settings_node, const Catalogue& tcat) {
        if (settings_node.IsNull()) return;
        SetSettings(settings_node);
        BuildGraph(tcat);
    }

    Router::Router(const json::Node& settings_node, graph::DirectedWeightedGraph<double> graph,
        std::map<std::string, graph::VertexId> stop_ids)
            : graph_(graph)
            , stop_ids_(stop_ids) {
        if (settings_node.IsNull()) return;
        SetSettings(settings_node);
        router_ptr_ = new graph::Router<double>(graph_);
    }

    void Router::SetGraph(graph::DirectedWeightedGraph<double>&& graph,
        std::map<std::string, graph::VertexId>&& stop_ids) {
        graph_ = move(graph);
        stop_ids_ = move(stop_ids);
        router_ptr_ = new graph::Router<double>(graph_);
    }

    const graph::DirectedWeightedGraph<double>& Router::BuildGraph(const Catalogue& tcat) {
        const map<string_view, Stop*>& all_stops = tcat.GetSortedAllStops();
        const map<string_view, Bus*>& all_buses = tcat.GetSortedAllBuses();
        graph::DirectedWeightedGraph<double> stops_graph(all_stops.size() * 2);
        map<std::string, graph::VertexId> stop_ids;
        graph::VertexId vertex_id = 0;
        for (const auto& [stop_name, stop_ptr] : all_stops) {
            stop_ids[stop_ptr->name] = vertex_id;
            stops_graph.AddEdge({ stop_ptr->name,
                                  0,
                                  vertex_id,
                                  ++vertex_id,
                                  static_cast<double>(bus_wait_time_) });
            ++vertex_id;
        }
        stop_ids_ = move(stop_ids);

        for_each(
            all_buses.begin(),
            all_buses.end(),
            [&stops_graph, this](const auto& item)
            {
                const auto& bus_ptr = item.second;
                const std::vector<Stop*>& stops = bus_ptr->stops;
                size_t stops_count = stops.size();
                for (size_t i = 0; i < stops_count; ++i) {
                    for (size_t j = i + 1; j < stops_count; ++j) {
                        const Stop* stop_from = stops[i];
                        const Stop* stop_to = stops[j];
                        int dist_sum = 0;
                        for (size_t k = i + 1; k <= j; ++k) {
                            dist_sum += stops[k - 1]->GetDistance(stops[k]);
                        }
                        stops_graph.AddEdge({ bus_ptr->name,
                                              j - i,
                                              stop_ids_.at(stop_from->name) + 1,
                                              stop_ids_.at(stop_to->name),
                                              static_cast<double>(dist_sum) / (bus_velocity_ * (100.0 / 6.0)) });
                        if (!bus_ptr->is_circle && stop_to == bus_ptr->final_stop && j == stops_count / 2) break;
                    }
                }
            });

        graph_ = move(stops_graph);
        router_ptr_ = new graph::Router<double>(graph_);
        return graph_;
    }

    json::Array Router::GetEdgesItems(const std::vector<graph::EdgeId>& edges) const {
        json::Array items_array;
        items_array.reserve(edges.size());
        for (auto& edge_id : edges) {
            const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
            if (edge.quality == 0) {
                items_array.emplace_back(json::Node(json::Dict{
                    {{"stop_name"s},{static_cast<string>(edge.name)}},
                    {{"time"s},{edge.weight}},
                    {{"type"s},{"Wait"s}}
                    }));
            }
            else {
                items_array.emplace_back(json::Node(json::Dict{
                    {{"bus"s},{static_cast<string>(edge.name)}},
                    {{"span_count"s},{static_cast<int>(edge.quality)}},
                    {{"time"s},{edge.weight}},
                    {{"type"s},{"Bus"s}}
                    }));
            }
        }
        return items_array;
    }

    std::optional<graph::Router<double>::RouteInfo> Router::GetRouteInfo(const Stop* from, const Stop* to) const {
        return router_ptr_->BuildRoute(stop_ids_.at(from->name), stop_ids_.at(to->name));
    }

    size_t Router::GetGraphVertexCount() {
        return graph_.GetVertexCount();
    }

    const std::map<std::string, graph::VertexId>& Router::GetStopIds() const {
        return stop_ids_;
    }

    const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
        return graph_;
    }

    json::Node Router::GetSettings() const {
        return json::Node(json::Dict{
            {{"bus_wait_time"s},{bus_wait_time_}},
            {{"bus_velocity"s},{bus_velocity_}}
            });
    }

    void Router::SetSettings(const json::Node& settings_node) {
        bus_wait_time_ = settings_node.AsDict().at("bus_wait_time"s).AsInt();
        bus_velocity_ = settings_node.AsDict().at("bus_velocity"s).AsDouble();
    }

} // namespace transport