#include "project47/algorithms/job_order.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    using project47::Job;
    using project47::JobStatus;

    std::vector<Job> jobs{
        {104, "tile_12_08", 2, 18.5, 8'400'000, JobStatus::Pending},
        {101, "tile_12_05", 5, 32.0, 14'200'000, JobStatus::Ready},
        {103, "tile_12_07", 5, 12.0, 6'700'000, JobStatus::Pending},
        {102, "tile_12_06", 3, 9.5, 4'100'000, JobStatus::Pending}
    };

    try {
        project47::sort_jobs_for_scheduling(jobs);
    } catch (const std::exception& error) {
        std::cerr << "Could not schedule jobs: " << error.what() << '\n';
        return 1;
    }

    std::cout << "Project 47 | Satellite Tile Processing Scheduler\n";
    std::cout << "Sorted jobs (priority desc, duration asc, id asc)\n\n";
    std::cout << std::left << std::setw(8) << "Job ID"
              << std::setw(16) << "Tile"
              << std::setw(10) << "Priority"
              << std::setw(14) << "Seconds"
              << std::setw(14) << "Input MB"
              << "Status\n";

    for (const Job& job : jobs) {
        std::cout << std::left << std::setw(8) << job.id
                  << std::setw(16) << job.tile_id
                  << std::setw(10) << job.priority
                  << std::setw(14) << std::fixed << std::setprecision(1) << job.estimated_seconds
                  << std::setw(14) << std::setprecision(2)
                  << (static_cast<double>(job.input_bytes) / 1'000'000.0)
                  << project47::to_string(job.status) << '\n';
    }

    return 0;
}
