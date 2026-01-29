/**
 * grid_calc.cpp
 * 
 * Command-line utility to calculate distance and bearing between
 * Maidenhead grid squares.
 * 
 * Compile:
 *   g++ -std=c++11 -O2 -o grid_calc grid_calc.cpp
 *   clang++ -std=c++11 -O2 -o grid_calc grid_calc.cpp
 * 
 * Usage:
 *   grid_calc GRID1 GRID2 [--unit km|mi|nm] [--verbose]
 * 
 * Examples:
 *   grid_calc FN42 JO01
 *   grid_calc FN42hn DM13at --unit mi
 *   grid_calc CN87 CN88 --verbose
 */

#include "gridsquare.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " GRID1 GRID2 [OPTIONS]\n\n"
              << "Calculate distance and bearing between Maidenhead grid squares\n\n"
              << "Arguments:\n"
              << "  GRID1               First grid square (e.g., FN42, FN42hn)\n"
              << "  GRID2               Second grid square\n\n"
              << "Options:\n"
              << "  --unit UNIT         Distance unit: km, mi, nm (default: km)\n"
              << "  -u UNIT             Short form of --unit\n"
              << "  --verbose           Show detailed information\n"
              << "  -v                  Short form of --verbose\n"
              << "  --help              Show this help message\n"
              << "  -h                  Short form of --help\n\n"
              << "Examples:\n"
              << "  " << programName << " FN42 JO01\n"
              << "  " << programName << " FN42hn DM13at --unit mi\n"
              << "  " << programName << " CN87 CN88 --verbose\n";
}

void printVerboseResult(const std::string& grid1, const std::string& grid2, 
                       const GridSquare::DistanceResult& result) {
    // Calculate distance in all units
    double distKm = GridSquare::distance(grid1, grid2, GridSquare::Unit::KILOMETERS);
    double distMi = GridSquare::distance(grid1, grid2, GridSquare::Unit::MILES);
    double distNm = GridSquare::distance(grid1, grid2, GridSquare::Unit::NAUTICAL_MILES);
    
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "From: " << std::left << std::setw(8) << grid1 
              << " (" << std::setw(8) << std::right << result.from.latitude << "°, "
              << std::setw(9) << result.from.longitude << "°)\n";
    std::cout << "To:   " << std::left << std::setw(8) << grid2
              << " (" << std::setw(8) << std::right << result.to.latitude << "°, "
              << std::setw(9) << result.to.longitude << "°)\n\n";
    
    std::cout << std::setprecision(1);
    std::cout << "Distance:\n";
    std::cout << "  " << std::setw(10) << distKm << " km\n";
    std::cout << "  " << std::setw(10) << distMi << " miles\n";
    std::cout << "  " << std::setw(10) << distNm << " nautical miles\n\n";
    
    std::cout << "Bearing:      " << std::setw(5) << result.bearing << "° (" 
              << GridSquare::bearingToDirection(result.bearing) << ")\n";
    std::cout << "Back Bearing: " << std::setw(5) << result.backBearing << "° (" 
              << GridSquare::bearingToDirection(result.backBearing) << ")\n";
}

void printSimpleResult(double distance, GridSquare::Unit unit) {
    std::cout << std::fixed << std::setprecision(1) 
              << distance << " " << GridSquare::unitToString(unit) << "\n";
}

void runExamples() {
    std::cout << "======================================================================\n";
    std::cout << "Maidenhead Grid Square Distance Calculator - C++ Version\n";
    std::cout << "======================================================================\n\n";
    std::cout << "Example Calculations:\n";
    std::cout << "----------------------------------------------------------------------\n";
    
    struct Example {
        std::string grid1;
        std::string grid2;
        std::string description;
    };
    
    Example examples[] = {
        {"FN42", "JO01", "Boston area to London area"},
        {"FN42hn", "DM13at", "Massachusetts to Arizona"},
        {"CN87", "CN88", "Adjacent grid squares"},
        {"JN25", "QF22", "Europe to Australia"}
    };
    
    for (const auto& ex : examples) {
        try {
            auto result = GridSquare::calculate(ex.grid1, ex.grid2, GridSquare::Unit::KILOMETERS);
            double distKm = result.distance;
            double distMi = GridSquare::distance(ex.grid1, ex.grid2, GridSquare::Unit::MILES);
            double distNm = GridSquare::distance(ex.grid1, ex.grid2, GridSquare::Unit::NAUTICAL_MILES);
            
            std::cout << "\n" << ex.description << "\n";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "  From: " << std::left << std::setw(8) << ex.grid1
                      << " (" << std::setw(7) << std::right << result.from.latitude << "°, "
                      << std::setw(8) << result.from.longitude << "°)\n";
            std::cout << "  To:   " << std::left << std::setw(8) << ex.grid2
                      << " (" << std::setw(7) << std::right << result.to.latitude << "°, "
                      << std::setw(8) << result.to.longitude << "°)\n";
            
            std::cout << std::setprecision(1);
            std::cout << "  Distance: " << distKm << " km (" 
                      << distMi << " mi, " << distNm << " nm)\n";
            std::cout << "  Bearing:  " << result.bearing << "° (" 
                      << GridSquare::bearingToDirection(result.bearing) << ")\n";
        } catch (const std::exception& e) {
            std::cout << "\n" << ex.description << ": Error - " << e.what() << "\n";
        }
    }
    
    std::cout << "\n======================================================================\n";
}

int main(int argc, char* argv[]) {
    // Show examples if no arguments
    if (argc == 1) {
        runExamples();
        std::cout << "\nFor command-line usage, run: " << argv[0] << " --help\n";
        return 0;
    }
    
    // Parse command-line arguments
    std::string grid1, grid2;
    GridSquare::Unit unit = GridSquare::Unit::KILOMETERS;
    bool verbose = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--unit" || arg == "-u") {
            if (i + 1 < argc) {
                std::string unitStr = argv[++i];
                if (unitStr == "km") {
                    unit = GridSquare::Unit::KILOMETERS;
                } else if (unitStr == "mi") {
                    unit = GridSquare::Unit::MILES;
                } else if (unitStr == "nm") {
                    unit = GridSquare::Unit::NAUTICAL_MILES;
                } else {
                    std::cerr << "Error: Unknown unit '" << unitStr << "'. Use km, mi, or nm.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --unit requires an argument\n";
                return 1;
            }
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            printUsage(argv[0]);
            return 1;
        } else {
            if (grid1.empty()) {
                grid1 = arg;
            } else if (grid2.empty()) {
                grid2 = arg;
            } else {
                std::cerr << "Error: Too many arguments\n";
                printUsage(argv[0]);
                return 1;
            }
        }
    }
    
    // Validate required arguments
    if (grid1.empty() || grid2.empty()) {
        std::cerr << "Error: Both GRID1 and GRID2 are required\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // Calculate and display results
    try {
        if (verbose) {
            auto result = GridSquare::calculate(grid1, grid2, unit);
            printVerboseResult(grid1, grid2, result);
        } else {
            double distance = GridSquare::distance(grid1, grid2, unit);
            printSimpleResult(distance, unit);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
