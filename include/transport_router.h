#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <vector>
#include <memory>

namespace transport_router {
    
    using RouteSettings = domain::RouteSettings;

    struct TravelDuration {
        TravelDuration() = default;
        TravelDuration(int stops_number, double waiting_time, double travel_time) 
            : stops_number(stops_number)
            , waiting_time(waiting_time)
            , travel_time(travel_time){
        }
        int stops_number = 0;
        double waiting_time = 0.0;
        double travel_time = 0.0;
    };

    TravelDuration operator+(const TravelDuration& lhs, const TravelDuration& rhs);

    bool operator<(const TravelDuration& lhs, const TravelDuration& rhs);

    bool operator>(const TravelDuration& lhs, const TravelDuration& rhs);

    struct TravelProps {
        domain::StopPtr from = nullptr;
        domain::StopPtr to = nullptr;
        domain::BusPtr route = nullptr;
        TravelDuration travel_duration = {};
    };

    class DistanceProcessor {
    public:
        DistanceProcessor(const catalogue::TransportCatalogue* catalogue, const domain::BusPtr route);
        size_t DistanceBetween(size_t index_from, size_t index_to);
    private:
        std::vector<size_t> forward_distance_;
        std::vector<size_t> reverse_distance_;
    };

    class TransportRouter {
    public:
        TransportRouter() = default;
        void InitRouter(RouteSettings settings, const catalogue::TransportCatalogue* catalogue);
        std::optional<std::vector<const TravelProps*>> FindRoute(std::string_view from, std::string_view to) const;
    private:
        RouteSettings settings_;
        const catalogue::TransportCatalogue* catalogue_ = nullptr;
        std::unique_ptr<graph::DirectedWeightedGraph<TravelDuration>> graph_ = nullptr;
        std::unordered_map<domain::StopPtr, graph::VertexId> graph_vertexes_ = {};
        std::vector<TravelProps> graph_edges_ = {};
        std::unique_ptr<graph::Router<TravelDuration>> router_;
    };
    
} // namespace transport_router