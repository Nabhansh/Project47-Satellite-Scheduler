#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace project47 {

enum class ComputeNodeStatus {
    Online,
    Busy,
    Offline,
    Maintenance
};

struct ComputeNode {
    std::string id;
    std::string hostname;
    std::uint32_t cpu_cores{};
    std::uint64_t memory_bytes{};
    std::uint64_t available_memory_bytes{};
    double processing_capacity_gflops{};
    ComputeNodeStatus status{ComputeNodeStatus::Online};

    void validate() const {
        if (id.empty() || hostname.empty()) {
            throw std::invalid_argument("Compute node id and hostname cannot be empty");
        }
        if (cpu_cores == 0 || memory_bytes == 0) {
            throw std::invalid_argument("Compute node must have CPU cores and memory");
        }
        if (available_memory_bytes > memory_bytes) {
            throw std::invalid_argument("Available memory cannot exceed total memory");
        }
        if (!std::isfinite(processing_capacity_gflops) || processing_capacity_gflops <= 0.0) {
            throw std::invalid_argument("Compute capacity must be finite and positive");
        }
    }
};

} // namespace project47
