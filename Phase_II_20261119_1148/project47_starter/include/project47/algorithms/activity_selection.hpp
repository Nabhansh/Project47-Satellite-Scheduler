#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>

namespace project47 {

// A processing window [start_time, finish_time) for a satellite tile job.
// Adjacent windows are compatible: one job may start when another finishes.
struct Activity {
    std::uint64_t id{};
    double start_time{};
    double finish_time{};

    void validate() const {
        if (id == 0) {
            throw std::invalid_argument("Activity ID must be non-zero");
        }
        if (!std::isfinite(start_time) || !std::isfinite(finish_time) ||
            start_time >= finish_time) {
            throw std::invalid_argument("Activity must have finite start < finish times");
        }
    }
};

inline void validate_activities(const std::vector<Activity>& activities) {
    std::set<std::uint64_t> ids;
    for (const auto& activity : activities) {
        activity.validate();
        if (!ids.insert(activity.id).second) {
            throw std::invalid_argument("Activity IDs must be unique");
        }
    }
}

// Greedy maximum-cardinality selection of non-overlapping processing windows.
// Runtime: O(n log n) for sorting, followed by O(n) selection.
inline std::vector<Activity> activity_selection(std::vector<Activity> activities) {
    validate_activities(activities);
    std::sort(activities.begin(), activities.end(), [](const Activity& a, const Activity& b) {
        if (a.finish_time != b.finish_time) return a.finish_time < b.finish_time;
        if (a.start_time != b.start_time) return a.start_time < b.start_time;
        return a.id < b.id;
    });

    std::vector<Activity> selected;
    selected.reserve(activities.size());
    double last_finish = -INFINITY;
    for (const auto& activity : activities) {
        if (activity.start_time >= last_finish) {
            selected.push_back(activity);
            last_finish = activity.finish_time;
        }
    }
    return selected;
}

} // namespace project47
