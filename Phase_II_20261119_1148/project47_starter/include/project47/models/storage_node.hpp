#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace project47 {

struct StorageNode {
    std::string id;
    std::string location;
    std::uint64_t capacity_bytes{};
    std::uint64_t used_bytes{};
    double bandwidth_mbps{};
    bool online{true};

    [[nodiscard]] std::uint64_t available_bytes() const noexcept {
        return capacity_bytes - used_bytes;
    }

    void validate() const {
        if (id.empty() || location.empty()) {
            throw std::invalid_argument("Storage node id and location cannot be empty");
        }
        if (capacity_bytes == 0 || used_bytes > capacity_bytes) {
            throw std::invalid_argument("Storage usage must be within a non-zero capacity");
        }
        if (!std::isfinite(bandwidth_mbps) || bandwidth_mbps <= 0.0) {
            throw std::invalid_argument("Storage bandwidth must be finite and positive");
        }
    }
};

} // namespace project47
