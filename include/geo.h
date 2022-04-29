#pragma once

#include <cmath>
#include <iostream>
#include <string>

namespace geo {

struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
};

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.;
    static const int earth_rd = 6371000;
    
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
        + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * earth_rd;
}

}  // namespace geo