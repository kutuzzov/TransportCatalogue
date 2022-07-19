#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

class JsonReader {
public:
    JsonReader(json::Document input_json)
        : input_(input_json) {}

    const json::Node& GetBaseRequest() const;

    const json::Node& GetStatRequest() const;

    const json::Node& GetRenderSettings() const;

    const json::Node& GetRoutingSettings() const;

    const json::Node& GetSerializationSettings() const;

    void FillCatalogue(transport::Catalogue& catalogue) const;

private:
    json::Document input_;
    json::Node dumm_{ nullptr };

    struct Bus_info {
        std::vector<std::string_view> stops;
        std::string_view final_stop;
        bool is_circle;
    };

    using StopsDistMap = std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>>;
    using BusesInfoMap = std::unordered_map<std::string_view, Bus_info>;

    void ParseStopAddRequest(transport::Catalogue& catalogue, const json::Dict& request_map,
        StopsDistMap& stop_to_stops_distance) const;
    void SetStopsDistances(transport::Catalogue& catalogue,
        const StopsDistMap& stop_to_stops_distance) const;
    void ParseBusAddRequest(const json::Dict& request_map, BusesInfoMap& buses_info) const;
    void BusesAddProcess(transport::Catalogue& catalogue, const BusesInfoMap& buses_info) const;
    void SetFinals(transport::Catalogue& catalogue, const BusesInfoMap& buses_info) const;
};