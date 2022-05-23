#pragma once
#include <algorithm>
#include <deque>
#include <numeric>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <string>
#include <string_view>
#include <functional>
#include <optional>

#include "domain.h"

namespace catalogue {

	using namespace domain;

	struct RouteInfo
	{
		std::string_view name;
		int stops = 0;
		int unique_stops = 0;
		int route_length = 0;
		double curvature = 0.0;
	};

	struct DistanceBetweenStopsHasher {
	public:
		size_t operator() (const std::pair<StopPtr, StopPtr> stops_pair) const {
			return hasher(stops_pair.first) + 37 * hasher(stops_pair.second);
		}
	private:
		std::hash<const void*> hasher;
	};

	class TransportCatalogue
	{
	public:
		void AddStop(const std::string_view name,
			geo::Coordinates coordinates,
			std::unordered_map<std::string, int>& stops_distance);
		void AddRoute(const std::string_view& name, const std::vector<std::string_view>& stops, bool is_circular);
		BusPtr FindRoute(const std::string_view& route_name) const;
		StopPtr FindStop(const std::string_view& stop_name) const;
		const std::unordered_set<BusPtr>* GetStopInfo(const std::string_view& stop_name) const;
		void SetDistanceBetweenStops(StopPtr from, StopPtr to, int distance);
		size_t GetDistanceBetweenStops(StopPtr from, StopPtr to) const;
		const std::map<std::string_view, BusPtr> GetAllRoutes() const;
		const std::unordered_map<std::string_view, StopPtr>* GetAllStops() const;
		const std::optional<RouteInfo> GetRouteInfo(const std::string_view& bus_name) const;

	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;

		std::unordered_map<std::string_view, BusPtr> buses_name_;
		std::unordered_map<std::string_view, StopPtr> stops_name_;
		std::unordered_map<StopPtr, std::unordered_set<BusPtr>> buses_in_stop_;
		std::unordered_map<std::pair<StopPtr, StopPtr>, double, DistanceBetweenStopsHasher> distance_between_stops_;
	};
}