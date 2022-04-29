#include "transport_catalogue.h"

namespace catalogue {

	void TransportCatalogue::AddRoute(const std::string_view& name,
		const std::vector<std::string_view>& stops,
		bool is_circular) {
		std::vector<const Stop*> route_stops;
		route_stops.reserve(stops.size());

		auto& route = buses_.emplace_back(Bus{ std::string(name), route_stops, is_circular });
		buses_name_.insert({ route.name, &route });
		for (const auto& stop : stops) {
			const auto* stop_ptr = FindStop(stop);
			route.stops.push_back(stop_ptr);
			buses_in_stop_[stop_ptr].insert(FindRoute(name));
		}
	}

	void TransportCatalogue::AddStop(const std::string_view name,
		geo::Coordinates coordinates,
		std::vector<std::pair<std::string, int>>& stops_distance) {
		const auto& stop = stops_.emplace_back(Stop{ std::string(name), coordinates,  stops_distance });
		stops_name_.insert({ stop.name, &stop });
		buses_in_stop_[&stop];
	}

	BusPtr TransportCatalogue::FindRoute(const std::string_view& route_name) const {
		const auto it = buses_name_.find(route_name);
		if (it == buses_name_.end()) {
			return nullptr;
		}
		else {
			return it->second;
		}
	}

	StopPtr TransportCatalogue::FindStop(const std::string_view& stop_name) const {
		const auto it = stops_name_.find(stop_name);
		if (it == stops_name_.end()) {
			return nullptr;
		}
		else {
			return it->second;
		}
	}

	const std::unordered_set<BusPtr>* TransportCatalogue::GetStopInfo(const std::string_view& stop_name) const {
		if (FindStop(stop_name)) {
			return &buses_in_stop_.at(FindStop(stop_name));
		}
		else {
			return nullptr;
		}
	}

	void TransportCatalogue::SetDistanceBetweenStops(StopPtr from, StopPtr to, int distance) {
		auto stops_pair = std::make_pair(from, to);
		distance_between_stops_[stops_pair] = distance;
	}

	size_t TransportCatalogue::GetDistanceBetweenStops(StopPtr from, StopPtr to) const {
		auto stops_pair = std::make_pair(from, to);
		if (distance_between_stops_.find(stops_pair) != distance_between_stops_.end()) {
			return distance_between_stops_.at(stops_pair);
		}
		return GetDistanceBetweenStops(to, from);
	}

	const std::map <std::string_view, BusPtr> TransportCatalogue::GetAllRoutes() const {
		std::map<std::string_view, BusPtr> result;
		for (const auto& route : buses_) {
			result[route.name] = &route;
		}
		return result;
	}

	const std::optional<RouteInfo> TransportCatalogue::GetRouteInfo(const std::string_view& bus_name) const {
		RouteInfo result;
		const auto* route_ptr = FindRoute(bus_name);
		if (route_ptr == nullptr) {
			return {};
		}

		std::vector<StopPtr> stops_local{ route_ptr->stops.begin(), route_ptr->stops.end() };
		std::unordered_set<StopPtr> unique_stops;

		std::for_each(stops_local.begin(), stops_local.end(), [&unique_stops](const auto& stop) {
			if (!unique_stops.count(stop)) {
				unique_stops.insert(stop);
			}
			});

		if (!route_ptr->is_circular) {
			stops_local.insert(stops_local.end(), next(route_ptr->stops.rbegin()), route_ptr->stops.rend());
		}

		double route_length_straight = 0;
		double route_length = 0;
		for (std::vector<StopPtr>::const_iterator it(stops_local.begin()); it != prev(stops_local.end()); ++it) {
			route_length_straight += ComputeDistance((*it)->coordinates, (*(it + 1))->coordinates);
			route_length += GetDistanceBetweenStops(*it, *(it + 1));
		}

		result.name = route_ptr->name;
		result.stops = static_cast<int>(stops_local.size());
		result.unique_stops = static_cast<int>(unique_stops.size());
		result.route_length = route_length;
		result.curvature = route_length / route_length_straight;
		return result;
	}
}