/**
 * gridsquare.hpp
 * 
 * Maidenhead Grid Square utilities for calculating distances and bearings
 * between ham radio grid locators.
 */

#ifndef GRIDSQUARE_HPP
#define GRIDSQUARE_HPP

#include <string>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <cctype>
#include <algorithm>

namespace GridSquare {

    /**
     * Earth radius in different units
     */
    constexpr double EARTH_RADIUS_KM = 6371.0;
    constexpr double EARTH_RADIUS_MI = 3959.0;
    constexpr double EARTH_RADIUS_NM = 3440.0;

    /**
     * Distance units
     */
    enum class Unit {
        KILOMETERS,
        MILES,
        NAUTICAL_MILES
    };

    /**
     * Coordinate pair (latitude, longitude) in decimal degrees
     */
    struct Coordinate {
        double latitude;
        double longitude;
        
        Coordinate() : latitude(0.0), longitude(0.0) {}
        Coordinate(double lat, double lon) : latitude(lat), longitude(lon) {}
    };

    /**
     * Result of distance/bearing calculation
     */
    struct DistanceResult {
        double distance;
        double bearing;
        double backBearing;
        Coordinate from;
        Coordinate to;
        
        DistanceResult() : distance(0.0), bearing(0.0), backBearing(0.0) {}
    };

    /**
     * Convert degrees to radians
     */
    inline double toRadians(double degrees) {
        return degrees * M_PI / 180.0;
    }

    /**
     * Convert radians to degrees
     */
    inline double toDegrees(double radians) {
        return radians * 180.0 / M_PI;
    }

    /**
     * Normalize angle to 0-360 range
     */
    inline double normalizeAngle(double angle) {
        angle = fmod(angle, 360.0);
        if (angle < 0) angle += 360.0;
        return angle;
    }

    /**
     * Convert Maidenhead grid square to latitude/longitude
     * 
     * @param grid Grid square string (2, 4, 6, or 8 characters)
     * @return Coordinate pair (center of grid square)
     * @throws std::invalid_argument if grid format is invalid
     */
    inline Coordinate toLatLon(std::string grid) {
        // Convert to uppercase and trim
        std::transform(grid.begin(), grid.end(), grid.begin(), ::toupper);
        
        size_t len = grid.length();
        if (len != 2 && len != 4 && len != 6 && len != 8) {
            throw std::invalid_argument("Grid square must be 2, 4, 6, or 8 characters");
        }

        // Validate format
        if (!std::isalpha(grid[0]) || !std::isalpha(grid[1])) {
            throw std::invalid_argument("First two characters must be letters");
        }
        if (len >= 4 && (!std::isdigit(grid[2]) || !std::isdigit(grid[3]))) {
            throw std::invalid_argument("Characters 3-4 must be digits");
        }
        if (len >= 6 && (!std::isalpha(grid[4]) || !std::isalpha(grid[5]))) {
            throw std::invalid_argument("Characters 5-6 must be letters");
        }
        if (len == 8 && (!std::isdigit(grid[6]) || !std::isdigit(grid[7]))) {
            throw std::invalid_argument("Characters 7-8 must be digits");
        }

        // Field (first 2 characters): 20° longitude, 10° latitude
        double lon = (grid[0] - 'A') * 20.0 - 180.0;
        double lat = (grid[1] - 'A') * 10.0 - 90.0;

        if (len >= 4) {
            // Square (characters 3-4): 2° longitude, 1° latitude
            lon += (grid[2] - '0') * 2.0;
            lat += (grid[3] - '0') * 1.0;
        }

        if (len >= 6) {
            // Subsquare (characters 5-6): 5' longitude, 2.5' latitude
            lon += (grid[4] - 'A') * (2.0 / 24.0);
            lat += (grid[5] - 'A') * (1.0 / 24.0);
        }

        if (len == 8) {
            // Extended square (characters 7-8): 30" longitude, 15" latitude
            lon += (grid[6] - '0') * (2.0 / 240.0);
            lat += (grid[7] - '0') * (1.0 / 240.0);
        }

        // Return center of the grid square
        if (len == 2) {
            lon += 10.0;  // Center of 20° field
            lat += 5.0;   // Center of 10° field
        } else if (len == 4) {
            lon += 1.0;   // Center of 2° square
            lat += 0.5;   // Center of 1° square
        } else if (len == 6) {
            lon += 1.0 / 24.0;  // Center of 5' subsquare
            lat += 1.0 / 48.0;  // Center of 2.5' subsquare
        } else if (len == 8) {
            lon += 1.0 / 240.0; // Center of 30" extended square
            lat += 1.0 / 480.0; // Center of 15" extended square
        }

        return Coordinate(lat, lon);
    }

    /**
     * Calculate great circle distance between two coordinates using Haversine formula
     * 
     * @param coord1 First coordinate
     * @param coord2 Second coordinate
     * @param unit Distance unit
     * @return Distance in specified units
     */
    inline double calculateDistance(const Coordinate& coord1, const Coordinate& coord2, Unit unit = Unit::KILOMETERS) {
        // Select Earth radius based on unit
        double R;
        switch (unit) {
            case Unit::KILOMETERS:
                R = EARTH_RADIUS_KM;
                break;
            case Unit::MILES:
                R = EARTH_RADIUS_MI;
                break;
            case Unit::NAUTICAL_MILES:
                R = EARTH_RADIUS_NM;
                break;
            default:
                R = EARTH_RADIUS_KM;
        }

        // Convert to radians
        double lat1 = toRadians(coord1.latitude);
        double lat2 = toRadians(coord2.latitude);
        double dLat = toRadians(coord2.latitude - coord1.latitude);
        double dLon = toRadians(coord2.longitude - coord1.longitude);

        // Haversine formula
        double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
                   std::cos(lat1) * std::cos(lat2) *
                   std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        return R * c;
    }

    /**
     * Calculate initial bearing from coord1 to coord2
     * 
     * @param coord1 Starting coordinate
     * @param coord2 Destination coordinate
     * @return Bearing in degrees (0-360, where 0/360 is North)
     */
    inline double calculateBearing(const Coordinate& coord1, const Coordinate& coord2) {
        // Convert to radians
        double lat1 = toRadians(coord1.latitude);
        double lat2 = toRadians(coord2.latitude);
        double dLon = toRadians(coord2.longitude - coord1.longitude);

        // Calculate bearing
        double x = std::sin(dLon) * std::cos(lat2);
        double y = std::cos(lat1) * std::sin(lat2) -
                   std::sin(lat1) * std::cos(lat2) * std::cos(dLon);

        double bearing = std::atan2(x, y);
        return normalizeAngle(toDegrees(bearing));
    }

    /**
     * Calculate distance between two grid squares
     * 
     * @param grid1 First grid square
     * @param grid2 Second grid square
     * @param unit Distance unit
     * @return Distance in specified units
     */
    inline double distance(const std::string& grid1, const std::string& grid2, Unit unit = Unit::KILOMETERS) {
        Coordinate coord1 = toLatLon(grid1);
        Coordinate coord2 = toLatLon(grid2);
        return calculateDistance(coord1, coord2, unit);
    }

    /**
     * Calculate bearing between two grid squares
     * 
     * @param grid1 Starting grid square
     * @param grid2 Destination grid square
     * @return Bearing in degrees (0-360)
     */
    inline double bearing(const std::string& grid1, const std::string& grid2) {
        Coordinate coord1 = toLatLon(grid1);
        Coordinate coord2 = toLatLon(grid2);
        return calculateBearing(coord1, coord2);
    }

    /**
     * Calculate complete distance and bearing information
     * 
     * @param grid1 First grid square
     * @param grid2 Second grid square
     * @param unit Distance unit
     * @return DistanceResult with all calculated values
     */
    inline DistanceResult calculate(const std::string& grid1, const std::string& grid2, Unit unit = Unit::KILOMETERS) {
        Coordinate coord1 = toLatLon(grid1);
        Coordinate coord2 = toLatLon(grid2);
        
        DistanceResult result;
        result.from = coord1;
        result.to = coord2;
        result.distance = calculateDistance(coord1, coord2, unit);
        result.bearing = calculateBearing(coord1, coord2);
        result.backBearing = calculateBearing(coord2, coord1);
        
        return result;
    }

    /**
     * Convert bearing to cardinal direction
     * 
     * @param bearing Bearing in degrees
     * @return Cardinal direction string (e.g., "N", "NE", "SSW")
     */
    inline std::string bearingToDirection(double bearing) {
        const char* directions[] = {
            "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
            "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
        };
        int index = static_cast<int>(std::round(bearing / 22.5)) % 16;
        return directions[index];
    }

    /**
     * Convert Unit enum to string
     */
    inline std::string unitToString(Unit unit) {
        switch (unit) {
            case Unit::KILOMETERS:
                return "km";
            case Unit::MILES:
                return "miles";
            case Unit::NAUTICAL_MILES:
                return "nm";
            default:
                return "km";
        }
    }

} // namespace GridSquare

#endif // GRIDSQUARE_HPP
