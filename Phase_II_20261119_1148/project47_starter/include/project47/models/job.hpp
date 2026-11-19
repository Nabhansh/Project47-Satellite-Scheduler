#pragma once

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace project47 {

enum class JobStatus {
    Pending,
    Ready,
    Running,
    Completed,
    Failed
};

struct Job {
    std::uint64_t id{};
    std::string tile_id;
    int priority{};                    // Higher values mean more urgent work.
    double estimated_seconds{};
    std::uint64_t input_bytes{};
    JobStatus status{JobStatus::Pending};

    void validate() const {
        if (id == 0) {
            throw std::invalid_argument("Job id must be non-zero");
        }
        if (tile_id.empty()) {
            throw std::invalid_argument("Job tile_id cannot be empty");
        }
        if (!std::isfinite(estimated_seconds) || estimated_seconds < 0.0) {
            throw std::invalid_argument("estimated_seconds must be finite and non-negative");
        }
    }
};

inline const char* to_string(JobStatus status) noexcept {
    switch (status) {
        case JobStatus::Pending: return "pending";
        case JobStatus::Ready: return "ready";
        case JobStatus::Running: return "running";
        case JobStatus::Completed: return "completed";
        case JobStatus::Failed: return "failed";
    }
    return "unknown";
}

} // namespace project47
