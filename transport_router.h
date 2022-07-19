#pragma once
#include "domain.h"
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <optional>
#include <string>
#include <string_view>

namespace transport {

    class Router {
    public:
        Router() = default;

        Router(const json::Node& settings_node);
        Router(const json::Node& settings_node, const Catalogue& tcat);
        Router(const json::Node& settings_node,
            graph::DirectedWeightedGraph<double> graph,
            std::map<std::string, graph::VertexId> stop_ids);

        void SetGraph(graph::DirectedWeightedGraph<double>&& graph,
            std::map<std::string, graph::VertexId>&& stop_ids);

        const graph::DirectedWeightedGraph<double>& BuildGraph(const Catalogue& tcat);

        json::Array GetEdgesItems(const std::vector<graph::EdgeId>& edges) const;

        std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(const Stop* from, const Stop* to) const;

        size_t GetGraphVertexCount();

        const std::map<std::string, graph::VertexId>& GetStopIds() const;

        const graph::DirectedWeightedGraph<double>& GetGraph() const;

        json::Node GetSettings() const;

        ~Router() {
            delete router_ptr_;
        }

    private:
        int bus_wait_time_ = 0;
        double bus_velocity_ = 0;

        graph::DirectedWeightedGraph<double> graph_;
        std::map<std::string, graph::VertexId> stop_ids_;

        graph::Router<double>* router_ptr_ = nullptr;

        void SetSettings(const json::Node& settings_node);
    };

} // namespace transport