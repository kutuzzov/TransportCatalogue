#include "json_reader.h"

using namespace std;

const json::Node& JsonReader::GetBaseRequest() const {
    if (input_.GetRoot().AsDict().count("base_requests"s))
        return input_.GetRoot().AsDict().at("base_requests"s);
    else return dumm_;
}

const json::Node& JsonReader::GetStatRequest() const {
    if (input_.GetRoot().AsDict().count("stat_requests"s))
        return input_.GetRoot().AsDict().at("stat_requests"s);
    else return dumm_;
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (input_.GetRoot().AsDict().count("render_settings"s))
        return input_.GetRoot().AsDict().at("render_settings"s);
    else return dumm_;
}

const json::Node& JsonReader::GetRoutingSettings() const {
    if (input_.GetRoot().AsDict().count("routing_settings"s))
        return input_.GetRoot().AsDict().at("routing_settings"s);
    else return dumm_;
}

const json::Node& JsonReader::GetSerializationSettings() const {
    if (input_.GetRoot().AsDict().count("serialization_settings"s))
        return input_.GetRoot().AsDict().at("serialization_settings"s);
    else return dumm_;
}

void JsonReader::FillCatalogue(transport::Catalogue& catalogue) const {
    const json::Array& arr = GetBaseRequest().AsArray();
    StopsDistMap stop_to_stops_distance;
    BusesInfoMap buses_info;
    for (const auto& request_node : arr) {
        const json::Dict& request_map = request_node.AsDict();
        const string& type = request_map.at("type"s).AsString();
        if (type == "Stop"s) {
            ParseStopAddRequest(catalogue, request_map, stop_to_stops_distance);
        }
        if (type == "Bus"s) {
            ParseBusAddRequest(request_map, buses_info);
        }
    }
    SetStopsDistances(catalogue, stop_to_stops_distance);
    BusesAddProcess(catalogue, buses_info);
    SetFinals(catalogue, buses_info);
}

void JsonReader::ParseStopAddRequest(transport::Catalogue& catalogue, const json::Dict& request_map,
    StopsDistMap& stop_to_stops_distance) const {
    const string& stop_name = request_map.at("name"s).AsString();
    catalogue.AddStop(stop_name, {
                    request_map.at("latitude"s).AsDouble(),
                    request_map.at("longitude"s).AsDouble() });
    const json::Dict& near_stops = request_map.at("road_distances"s).AsDict();
    for (const auto& [key_stop_name, dist_node] : near_stops) {
        stop_to_stops_distance[stop_name][key_stop_name] = dist_node.AsInt();
    }
}

void JsonReader::SetStopsDistances(transport::Catalogue& catalogue,
    const StopsDistMap& stop_to_stops_distance) const {
    for (const auto& [stop, near_stops] : stop_to_stops_distance) {
        for (const auto& [stop_name, dist] : near_stops) {
            catalogue.SetDistance(catalogue.FindStop(stop), catalogue.FindStop(stop_name), dist);
        }
    }
}

void JsonReader::ParseBusAddRequest(const json::Dict& request_map, BusesInfoMap& buses_info) const {
    const string& bus_name = request_map.at("name"s).AsString();
    const json::Array& bus_stops = request_map.at("stops"s).AsArray();
    size_t stops_count = bus_stops.size();
    bool is_roundtrip = request_map.at("is_roundtrip"s).AsBool();
    buses_info[bus_name].is_circle = is_roundtrip;
    auto& stops = buses_info[bus_name].stops;
    if (stops_count > 0) stops.reserve(is_roundtrip ? stops_count : stops_count * 2);
    for (size_t i = 0; i < bus_stops.size(); ++i) {
        stops.push_back(bus_stops[i].AsString());
        if (i == bus_stops.size() - 1) {
            if (!is_roundtrip) {
                buses_info[bus_name].final_stop = bus_stops[i].AsString();
                for (int j = stops.size() - 2; j >= 0; --j) {
                    stops.push_back(stops[j]);
                }
            }
            else {
                buses_info[bus_name].final_stop = bus_stops[0].AsString();
            }
        }
    }
}

void JsonReader::BusesAddProcess(transport::Catalogue& catalogue, const BusesInfoMap& buses_info) const {
    for (const auto& [name, info] : buses_info) {
        vector<transport::Stop*> stop_ptrs;
        const auto& stops = info.stops;
        stop_ptrs.reserve(stops.size());
        for (const auto& stop : stops) {
            stop_ptrs.push_back(catalogue.FindStop(stop));
        }
        catalogue.AddBus(static_cast<string>(name), stop_ptrs, info.is_circle);
    }
}

void JsonReader::SetFinals(transport::Catalogue& catalogue, const BusesInfoMap& buses_info) const {
    for (auto& [bus_name, info] : buses_info) {
        if (domain::Bus* bus = catalogue.FindBus(bus_name)) {
            if (domain::Stop* stop = catalogue.FindStop(info.final_stop)) {
                bus->final_stop = stop;
            }
        }
    }
}