#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace project47 {

struct GeoBounds {
    double min_longitude{};
    double min_latitude{};
    double max_longitude{};
    double max_latitude{};

    void validate() const {
        if (!std::isfinite(min_longitude) || !std::isfinite(max_longitude) ||
            !std::isfinite(min_latitude) || !std::isfinite(max_latitude)) {
            throw std::invalid_argument("Tile bounds must contain finite coordinates");
        }
        if (min_longitude < -180.0 || max_longitude > 180.0 ||
            min_latitude < -90.0 || max_latitude > 90.0) {
            throw std::invalid_argument("Tile bounds are outside valid geographic ranges");
        }
        if (min_longitude >= max_longitude || min_latitude >= max_latitude) {
            throw std::invalid_argument("Tile bounds must have positive width and height");
        }
    }
};

// A tile uses the standard XYZ/Web-Mercator addressing convention.
// At zoom z, both column (x) and row (y) must be in [0, 2^z).
struct Tile {
    std::string id;
    std::uint8_t zoom_level{};
    std::uint32_t column{};
    std::uint32_t row{};
    GeoBounds bounds;
    double ground_resolution_m{};
    std::uint64_t byte_size{};
    std::string checksum;

    void validate() const {
        if (id.empty()) {
            throw std::invalid_argument("Tile id cannot be empty");
        }
        if (zoom_level > 30) {
            throw std::invalid_argument("Tile zoom level must be between 0 and 30");
        }
        const std::uint32_t dimension = std::uint32_t{1} << zoom_level;
        if (column >= dimension || row >= dimension) {
            throw std::invalid_argument("Tile column or row is outside its zoom-level grid");
        }
        bounds.validate();
        if (!std::isfinite(ground_resolution_m) || ground_resolution_m <= 0.0) {
            throw std::invalid_argument("Tile ground resolution must be finite and positive");
        }
        if (byte_size == 0) {
            throw std::invalid_argument("Tile byte_size must be non-zero");
        }
    }
};

} // namespace project47
