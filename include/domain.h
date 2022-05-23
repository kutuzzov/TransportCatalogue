#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace domain {

	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates;
		std::unordered_map<std::string, int> stops_distance;
	};

	using StopPtr = const Stop*;

	struct Bus
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_circular;
	};

	using BusPtr = const Bus*;

	struct RouteSettings
	{
		int bus_wait_time = 1;
		double bus_velocity = 1.0;
	};

} // namespace Domain