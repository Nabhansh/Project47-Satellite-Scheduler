#pragma once

#include "project47/algorithms/activity_selection.hpp"

#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace project47 {

struct IntervalAssignment {
    Activity activity;
    std::size_t resource_id{}; // 1-based compute-slot / worker identifier
};

struct IntervalPartitionResult {
    std::size_t resource_count{};
    std::vector<IntervalAssignment> assignments;
};

// Assigns every interval to the minimum number of identical resources.
// Runtime: O(n log n), using a min-heap of active resource finish times.
inline IntervalPartitionResult interval_partitioning(std::vector<Activity> activities) {
    validate_activities(activities);
    std::sort(activities.begin(), activities.end(), [](const Activity& a, const Activity& b) {
        if (a.start_time != b.start_time) return a.start_time < b.start_time;
        if (a.finish_time != b.finish_time) return a.finish_time < b.finish_time;
        return a.id < b.id;
    });

    using ActiveResource = std::pair<double, std::size_t>; // finish time, resource ID
    std::priority_queue<ActiveResource, std::vector<ActiveResource>,
                        std::greater<ActiveResource>> active;
    std::priority_queue<std::size_t, std::vector<std::size_t>,
                        std::greater<std::size_t>> reusable;

    IntervalPartitionResult result;
    for (const auto& activity : activities) {
        while (!active.empty() && active.top().first <= activity.start_time) {
            reusable.push(active.top().second);
            active.pop();
        }

        std::size_t resource_id;
        if (!reusable.empty()) {
            resource_id = reusable.top();
            reusable.pop();
        } else {
            resource_id = ++result.resource_count;
        }

        result.assignments.push_back({activity, resource_id});
        active.emplace(activity.finish_time, resource_id);
    }
    return result;
}

} // namespace project47
