#pragma once

#include "project47/algorithms/merge_sort.hpp"
#include "project47/models/job.hpp"

#include <vector>

namespace project47 {

// Scheduling order: highest priority first, then shortest estimated processing
// time, then lowest job id for deterministic output.
struct JobSchedulingOrder {
    bool operator()(const Job& a, const Job& b) const noexcept {
        if (a.priority != b.priority) {
            return a.priority > b.priority;
        }
        if (a.estimated_seconds != b.estimated_seconds) {
            return a.estimated_seconds < b.estimated_seconds;
        }
        return a.id < b.id;
    }
};

inline void sort_jobs_for_scheduling(std::vector<Job>& jobs) {
    for (const Job& job : jobs) {
        job.validate();
    }
    merge_sort(jobs, JobSchedulingOrder{});
}

} // namespace project47
