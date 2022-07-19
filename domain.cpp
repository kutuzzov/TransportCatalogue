#include "domain.h"
#include <stdexcept>

namespace domain {

    Stop::Stop(const std::string& name, const geo::Coordinates& coordinates)
        : name(name)
        , coordinates(coordinates) {}

    int Stop::GetDistance(Stop* to) {
        if (stop_distances.count(to->name)) return stop_distances.at(to->name);
        else if (to->stop_distances.count(this->name)) return to->stop_distances.at(this->name);
        else return 0;
    }

    Bus::Bus(const std::string& name, std::vector<Stop*> stops, bool is_circle)
        : name(name)
        , stops(stops)
        , is_circle(is_circle)
    {}

} //namespace domain