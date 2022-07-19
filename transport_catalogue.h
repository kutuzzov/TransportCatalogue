#pragma once

#include "geo.h"
#include "domain.h"

#include <deque>
#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>
#include <map>

namespace transport {

    using namespace domain;

    class Catalogue {
    public:
        void AddStop(const std::string& name, const geo::Coordinates& coordinates);

        void AddBus(const std::string& num, const std::vector<Stop*>& stops, bool is_circle);

        Stop* FindStop(const std::string_view stop);

        const Stop* FindStop(const std::string_view stop) const;

        Bus* FindBus(const std::string_view bus_num);

        const Bus* FindBus(const std::string_view bus_num) const;

        std::map<std::string_view, Bus*> GetBusesOnStop(const std::string_view stop_name);

        const std::map<std::string_view, Bus*> GetBusesOnStop(const std::string_view stop_name) const;

        void SetDistance(Stop* from, Stop* to, int dist);

        int GetDistance(const Stop* from, const Stop* to) const;

        const std::map <std::string_view, Bus*>& GetSortedAllBuses() const;

        const std::map <std::string_view, Stop*>& GetSortedAllStops() const;

    private:
        std::deque<Stop> all_stops_;
        std::deque<Bus> all_buses_;
        std::unordered_map < std::string_view, std::map<std::string_view, Bus*>> stop_to_buses_;
        std::map < std::string_view, Stop* > stops_list_;
        std::map < std::string_view, Bus* > buses_list_;
    };
    
} // namespace transport