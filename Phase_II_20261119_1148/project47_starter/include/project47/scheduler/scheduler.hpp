#pragma once

#include "project47/algorithms/job_order.hpp"
#include "project47/graphs/bipartite_matching.hpp"
#include "project47/graphs/dependency_graph.hpp"
#include "project47/graphs/shortest_paths.hpp"
#include "project47/models/compute_node.hpp"
#include "project47/models/job.hpp"
#include "project47/models/storage_node.hpp"
#include "project47/models/tile.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace project47 {

enum class Scenario {
    Normal,
    CapacityShock,
    Stress
};

inline const char* to_string(Scenario scenario) noexcept {
    switch (scenario) {
        case Scenario::Normal: return "normal";
        case Scenario::CapacityShock: return "capacity-shock";
        case Scenario::Stress: return "stress";
    }
    return "unknown";
}

struct SchedulerWorkload {
    std::vector<Job> jobs;
    std::vector<Tile> tiles;
    std::vector<ComputeNode> compute_nodes;
    std::vector<StorageNode> storage_nodes;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> dependencies;
    std::string input_storage_id;
    std::string output_storage_id;
    WeightedGraph network;
};

struct JobExecution {
    Job job;
    std::string compute_node_id;
    std::string source_storage_id;
    std::string destination_storage_id;
    std::size_t wave{};
    double route_cost{};
    std::vector<std::string> input_route;
    std::vector<std::string> output_route;
};

struct SchedulerResult {
    std::string scenario;
    std::size_t total_jobs{};
    std::size_t waves{};
    std::size_t matching_assignments{};
    double total_estimated_processing_seconds{};
    double estimated_makespan_seconds{};
    double total_route_cost{};
    std::vector<JobExecution> completed_jobs;
    std::vector<Job> deferred_jobs;
    std::map<std::string, std::uint64_t> final_storage_used_bytes;

    [[nodiscard]] std::size_t completed_count() const noexcept {
        return completed_jobs.size();
    }
    [[nodiscard]] std::size_t deferred_count() const noexcept {
        return deferred_jobs.size();
    }
};

namespace detail {

inline std::string matching_job_key(std::size_t rank) {
    // Fixed-width decimal keys preserve scheduler-priority order in the matching set.
    std::string key = std::to_string(rank);
    if (key.size() < 20) key.insert(key.begin(), 20 - key.size(), '0');
    return key;
}

struct RoutePlan {
    std::string source_storage;
    std::string destination_storage;
    std::vector<std::string> input_route;
    std::vector<std::string> output_route;
    double cost{};
};

struct WorkerRoute {
    std::vector<std::string> input_route;
    std::vector<std::string> output_route;
    double input_cost{};
    double output_cost{};
};

// Network topology is immutable during one scheduler run. Compute all shortest
// paths once: one search from input storage shared by every worker, plus one
// reverse-direction search from each online worker to the output storage.
// This changes route planning from repeated job-worker Dijkstra calls to
// O((W + 1) * (V + E) log V) per workload, where W is online workers.
struct RouteCache {
    std::map<std::string, WorkerRoute> by_compute_node;
    std::size_t dijkstra_runs{};
};

inline RouteCache build_route_cache(const SchedulerWorkload& workload) {
    RouteCache cache;
    const auto source = std::find_if(workload.storage_nodes.begin(), workload.storage_nodes.end(),
        [&](const StorageNode& node) { return node.id == workload.input_storage_id; });
    const auto destination = std::find_if(workload.storage_nodes.begin(), workload.storage_nodes.end(),
        [&](const StorageNode& node) { return node.id == workload.output_storage_id; });
    if (source == workload.storage_nodes.end() || destination == workload.storage_nodes.end()) {
        throw std::invalid_argument("Route cache requires known input and output storage nodes");
    }

    const ShortestPathResult from_source = dijkstra(workload.network, source->id);
    ++cache.dijkstra_runs;

    for (const ComputeNode& compute : workload.compute_nodes) {
        if (compute.status != ComputeNodeStatus::Online) continue;

        WorkerRoute route;
        const auto to_compute = from_source.distance.find(compute.id);
        if (to_compute != from_source.distance.end() && std::isfinite(to_compute->second)) {
            route.input_cost = to_compute->second;
            route.input_route = from_source.path_to(compute.id);
        }

        const ShortestPathResult from_compute = dijkstra(workload.network, compute.id);
        ++cache.dijkstra_runs;
        const auto to_destination = from_compute.distance.find(destination->id);
        if (to_destination != from_compute.distance.end() && std::isfinite(to_destination->second)) {
            route.output_cost = to_destination->second;
            route.output_route = from_compute.path_to(destination->id);
        }
        cache.by_compute_node.emplace(compute.id, std::move(route));
    }
    return cache;
}

inline bool find_route_plan(const Job& job,
                            const ComputeNode& compute,
                            const std::vector<StorageNode>& storage_nodes,
                            const std::string& input_storage_id,
                            const std::string& output_storage_id,
                            const RouteCache& route_cache,
                            RoutePlan& plan) {
    const auto source = std::find_if(storage_nodes.begin(), storage_nodes.end(),
        [&](const StorageNode& node) { return node.id == input_storage_id; });
    const auto destination = std::find_if(storage_nodes.begin(), storage_nodes.end(),
        [&](const StorageNode& node) { return node.id == output_storage_id; });
    if (source == storage_nodes.end() || destination == storage_nodes.end() ||
        !source->online || !destination->online || source->used_bytes < job.input_bytes ||
        destination->available_bytes() < job.input_bytes) {
        return false;
    }

    const auto cached = route_cache.by_compute_node.find(compute.id);
    if (cached == route_cache.by_compute_node.end()) return false;
    const WorkerRoute& route = cached->second;
    if (route.input_route.empty() || route.output_route.empty()) return false;

    const double cost = route.input_cost + route.output_cost;
    if (!std::isfinite(cost)) return false;
    plan.source_storage = source->id;
    plan.destination_storage = destination->id;
    plan.input_route = route.input_route;
    plan.output_route = route.output_route;
    plan.cost = cost;
    return true;
}

inline void validate_workload(const SchedulerWorkload& workload) {
    if (workload.jobs.empty()) throw std::invalid_argument("Workload must contain at least one job");
    if (workload.tiles.empty()) throw std::invalid_argument("Workload must contain at least one tile");
    if (workload.compute_nodes.empty()) throw std::invalid_argument("Workload must contain compute nodes");
    if (workload.storage_nodes.size() < 2) {
        throw std::invalid_argument("At least two storage nodes are required for input and output transfers");
    }

    std::set<std::uint64_t> job_ids;
    std::set<std::string> tile_ids;
    std::set<std::string> compute_ids;
    std::set<std::string> storage_ids;
    std::map<std::string, std::uint64_t> tile_sizes;

    for (const Tile& tile : workload.tiles) {
        tile.validate();
        if (!tile_ids.insert(tile.id).second) throw std::invalid_argument("Tile IDs must be unique");
        tile_sizes.emplace(tile.id, tile.byte_size);
    }
    for (const Job& job : workload.jobs) {
        job.validate();
        if (!job_ids.insert(job.id).second) throw std::invalid_argument("Job IDs must be unique");
        if (job.status != JobStatus::Pending && job.status != JobStatus::Ready) {
            throw std::invalid_argument("Input jobs must be pending or ready");
        }
        const auto tile = tile_sizes.find(job.tile_id);
        if (tile == tile_sizes.end()) throw std::invalid_argument("Every job must reference a known tile");
        if (tile->second != job.input_bytes) {
            throw std::invalid_argument("Job input_bytes must match its tile byte_size");
        }
    }
    for (const ComputeNode& node : workload.compute_nodes) {
        node.validate();
        if (!compute_ids.insert(node.id).second) throw std::invalid_argument("Compute node IDs must be unique");
        if (!workload.network.contains_vertex(node.id)) {
            throw std::invalid_argument("Every compute node must exist in the transfer network");
        }
    }
    for (const StorageNode& node : workload.storage_nodes) {
        node.validate();
        if (!storage_ids.insert(node.id).second || compute_ids.contains(node.id)) {
            throw std::invalid_argument("Storage and compute node IDs must be unique across infrastructure");
        }
        if (!workload.network.contains_vertex(node.id)) {
            throw std::invalid_argument("Every storage node must exist in the transfer network");
        }
    }
    if (workload.input_storage_id.empty() || workload.output_storage_id.empty() ||
        workload.input_storage_id == workload.output_storage_id ||
        !storage_ids.contains(workload.input_storage_id) || !storage_ids.contains(workload.output_storage_id)) {
        throw std::invalid_argument("Workload must identify distinct, known input and output storage nodes");
    }
    for (const auto& [prerequisite, dependent] : workload.dependencies) {
        if (!job_ids.contains(prerequisite) || !job_ids.contains(dependent)) {
            throw std::invalid_argument("Dependency references a job outside the workload");
        }
    }

    DependencyGraph graph;
    for (const std::uint64_t id : job_ids) graph.add_job(id);
    for (const auto& [prerequisite, dependent] : workload.dependencies) {
        (void)graph.add_dependency(prerequisite, dependent);
    }
    (void)graph.topological_order(); // Also rejects dependency cycles.
}

} // namespace detail

// Dependency-aware, wave-based batch scheduler. Each wave uses maximum bipartite
// matching to assign ready jobs to distinct online workers; Dijkstra selects an
// input-storage -> worker -> output-storage route. A job is marked complete only
// after resource and destination-capacity checks succeed.
inline SchedulerResult run_scheduler(SchedulerWorkload workload, Scenario scenario = Scenario::Normal) {
    detail::validate_workload(workload);
    const detail::RouteCache route_cache = detail::build_route_cache(workload);

    SchedulerResult result;
    result.scenario = to_string(scenario);
    result.total_jobs = workload.jobs.size();

    DependencyGraph dependency_graph;
    std::map<std::uint64_t, std::vector<std::uint64_t>> prerequisites;
    for (const Job& job : workload.jobs) dependency_graph.add_job(job.id);
    for (const auto& [prerequisite, dependent] : workload.dependencies) {
        if (dependency_graph.add_dependency(prerequisite, dependent)) {
            prerequisites[dependent].push_back(prerequisite);
        }
    }
    (void)dependency_graph.topological_order();

    std::map<std::uint64_t, Job> pending;
    for (const Job& job : workload.jobs) pending.emplace(job.id, job);
    std::set<std::uint64_t> completed;

    while (!pending.empty()) {
        std::vector<Job> ready_jobs;
        for (const auto& [id, job] : pending) {
            const auto deps = prerequisites.find(id);
            const bool prerequisites_done = deps == prerequisites.end() ||
                std::all_of(deps->second.begin(), deps->second.end(),
                            [&](std::uint64_t prerequisite) { return completed.contains(prerequisite); });
            if (prerequisites_done) ready_jobs.push_back(job);
        }
        if (ready_jobs.empty()) break; // Remaining jobs depend on work that could not be scheduled.
        sort_jobs_for_scheduling(ready_jobs);

        BipartiteGraph eligibility;
        std::map<std::string, std::uint64_t> key_to_job;
        std::map<std::uint64_t, std::string> job_to_key;
        for (std::size_t rank = 0; rank < ready_jobs.size(); ++rank) {
            const Job& job = ready_jobs[rank];
            const std::string key = detail::matching_job_key(rank);
            eligibility.add_left_vertex(key);
            key_to_job.emplace(key, job.id);
            job_to_key.emplace(job.id, key);
        }
        for (const ComputeNode& compute : workload.compute_nodes) {
            if (compute.status != ComputeNodeStatus::Online) continue;
            eligibility.add_right_vertex(compute.id);
            for (const Job& job : ready_jobs) {
                if (compute.available_memory_bytes < job.input_bytes) continue;
                detail::RoutePlan route;
                if (detail::find_route_plan(job, compute, workload.storage_nodes, workload.input_storage_id,
                                              workload.output_storage_id, route_cache, route)) {
                    (void)eligibility.add_edge(job_to_key.at(job.id), compute.id);
                }
            }
        }

        const BipartiteMatchingResult matching = eligibility.maximum_matching();
        if (matching.pairs.empty()) break;

        std::map<std::uint64_t, std::string> assignment;
        for (const MatchingPair& pair : matching.pairs) {
            assignment.emplace(key_to_job.at(pair.left), pair.right);
        }

        bool made_progress = false;
        double wave_max_seconds = 0.0;
        const std::size_t current_wave = result.waves + 1;
        for (const Job& job : ready_jobs) {
            const auto assigned = assignment.find(job.id);
            if (assigned == assignment.end()) continue;
            const auto compute_it = std::find_if(workload.compute_nodes.begin(), workload.compute_nodes.end(),
                [&](const ComputeNode& node) { return node.id == assigned->second; });
            if (compute_it == workload.compute_nodes.end()) throw std::logic_error("Matched compute node disappeared");

            detail::RoutePlan route;
            if (!detail::find_route_plan(job, *compute_it, workload.storage_nodes, workload.input_storage_id,
                                          workload.output_storage_id, route_cache, route)) {
                continue; // A prior assignment in this wave may have consumed the last output capacity.
            }
            auto destination_it = std::find_if(workload.storage_nodes.begin(), workload.storage_nodes.end(),
                [&](const StorageNode& node) { return node.id == route.destination_storage; });
            if (destination_it == workload.storage_nodes.end() ||
                destination_it->available_bytes() < job.input_bytes) {
                continue;
            }
            if (destination_it->used_bytes > std::numeric_limits<std::uint64_t>::max() - job.input_bytes) {
                throw std::overflow_error("Destination storage usage overflows uint64_t");
            }

            destination_it->used_bytes += job.input_bytes;
            Job completed_job = job;
            completed_job.status = JobStatus::Completed;
            result.completed_jobs.push_back({completed_job, compute_it->id,
                route.source_storage, route.destination_storage, current_wave,
                route.cost, std::move(route.input_route), std::move(route.output_route)});
            result.total_estimated_processing_seconds += job.estimated_seconds;
            wave_max_seconds = std::max(wave_max_seconds, job.estimated_seconds);
            result.total_route_cost += route.cost;
            result.matching_assignments += 1;
            completed.insert(job.id);
            pending.erase(job.id);
            made_progress = true;
        }
        if (!made_progress) break;
        // This implementation is barrier-synchronized: the next scheduling wave
        // is considered only after the current wave is committed. Summing each
        // wave's maximum duration is therefore a conservative wave-barrier
        // estimate, not an event-driven optimal makespan; an asynchronous
        // scheduler could overlap independent jobs across nominal wave levels.
        result.estimated_makespan_seconds += wave_max_seconds;
        result.waves = current_wave;
    }

    for (const auto& [id, job] : pending) {
        (void)id;
        result.deferred_jobs.push_back(job);
    }
    std::sort(result.deferred_jobs.begin(), result.deferred_jobs.end(),
              [](const Job& a, const Job& b) { return a.id < b.id; });
    for (const StorageNode& storage : workload.storage_nodes) {
        result.final_storage_used_bytes.emplace(storage.id, storage.used_bytes);
    }
    return result;
}

inline SchedulerWorkload make_demo_workload(Scenario scenario = Scenario::Normal) {
    SchedulerWorkload workload;
    const std::uint64_t mb = 1'000'000;
    const std::vector<std::tuple<std::uint64_t, const char*, int, double, std::uint64_t>> specs{
        {101, "tile_12_05", 5, 10.0, 10 * mb},
        {102, "tile_12_06", 4, 12.0, 8 * mb},
        {103, "tile_12_07", 5, 7.0, 6 * mb},
        {104, "tile_12_08", 3, 15.0, 12 * mb},
        {105, "tile_12_09", 2, 9.0, 9 * mb}
    };
    std::uint32_t x = 1200;
    std::uint32_t y = 1500;
    for (const auto& [id, tile_id, priority, seconds, bytes] : specs) {
        workload.jobs.push_back({id, tile_id, priority, seconds, bytes, JobStatus::Pending});
        workload.tiles.push_back({tile_id, 12, x++, y++, {-122.50, 37.70, -122.40, 37.80},
                                   4.78, bytes, "sha256:demo"});
    }
    workload.dependencies = {{101, 104}, {102, 104}, {103, 105}};

    const std::uint64_t memory = 64ULL * 1024 * 1024;
    workload.compute_nodes = {
        {"compute-01", "worker-01.local", 16, memory, memory, 850.0, ComputeNodeStatus::Online},
        {"compute-02", "worker-02.local", 16, memory, memory, 790.0, ComputeNodeStatus::Online},
        {"compute-03", "worker-03.local", 8, memory / 2, memory / 2, 520.0, ComputeNodeStatus::Online}
    };
    const std::uint64_t storage_capacity = 2ULL * 1024 * 1024 * 1024;
    workload.storage_nodes = {
        {"storage-source", "region-west", storage_capacity, storage_capacity / 2, 1200.0, true},
        {"storage-archive", "region-east", storage_capacity, 0, 900.0, true}
    };
    workload.input_storage_id = "storage-source";
    workload.output_storage_id = "storage-archive";

    for (const ComputeNode& node : workload.compute_nodes) workload.network.add_vertex(node.id);
    for (const StorageNode& node : workload.storage_nodes) workload.network.add_vertex(node.id);
    workload.network.add_edge("storage-source", "compute-01", 1.0);
    workload.network.add_edge("storage-source", "compute-02", 1.8);
    workload.network.add_edge("storage-source", "compute-03", 2.1);
    workload.network.add_edge("compute-01", "storage-archive", 1.5);
    workload.network.add_edge("compute-02", "storage-archive", 1.0);
    workload.network.add_edge("compute-03", "storage-archive", 1.2);
    workload.network.add_edge("compute-01", "compute-02", 0.8);
    workload.network.add_edge("compute-02", "compute-03", 0.6);

    if (scenario == Scenario::CapacityShock) {
        workload.compute_nodes[1].status = ComputeNodeStatus::Offline;
        workload.compute_nodes[2].status = ComputeNodeStatus::Offline;
        // Only 20 MB of output capacity remains, although the input dataset is unchanged.
        workload.storage_nodes[1].used_bytes = storage_capacity - 20 * mb;
    }
    return workload;
}

inline SchedulerWorkload make_stress_workload(std::size_t job_count = 128) {
    if (job_count == 0) throw std::invalid_argument("Stress workload must contain at least one job");
    const std::uint64_t mb = 1'000'000;
    const std::uint64_t maximum_jobs =
        (std::numeric_limits<std::uint64_t>::max() - mb) / (8 * mb);
    if (job_count > maximum_jobs) throw std::length_error("Stress workload storage size would overflow");
    SchedulerWorkload workload;
    const std::uint64_t memory = 128ULL * mb;
    constexpr std::size_t worker_count = 8;
    workload.compute_nodes.reserve(worker_count);
    for (std::size_t i = 0; i < worker_count; ++i) {
        const std::string id = "stress-compute-" + std::to_string(i + 1);
        workload.compute_nodes.push_back({id, id + ".local", 16, memory, memory,
            700.0 + static_cast<double>(i * 10), ComputeNodeStatus::Online});
        workload.network.add_vertex(id);
    }
    const std::uint64_t total_bytes = static_cast<std::uint64_t>(job_count) * 8 * mb;
    const std::uint64_t storage_capacity = total_bytes * 2 + 1'000'000;
    workload.storage_nodes = {
        {"stress-source", "stress-region-a", storage_capacity, storage_capacity, 5000.0, true},
        {"stress-archive", "stress-region-b", storage_capacity, 0, 5000.0, true}
    };
    workload.input_storage_id = "stress-source";
    workload.output_storage_id = "stress-archive";
    workload.network.add_vertex("stress-source");
    workload.network.add_vertex("stress-archive");
    for (std::size_t i = 0; i < worker_count; ++i) {
        const std::string id = workload.compute_nodes[i].id;
        workload.network.add_edge("stress-source", id, 1.0 + static_cast<double>(i) * 0.1);
        workload.network.add_edge(id, "stress-archive", 1.0 + static_cast<double>(i) * 0.1);
    }
    workload.jobs.reserve(job_count);
    workload.tiles.reserve(job_count);
    for (std::size_t i = 0; i < job_count; ++i) {
        const std::uint64_t id = 10'000 + static_cast<std::uint64_t>(i);
        const std::uint64_t bytes = (2 + static_cast<std::uint64_t>(i % 7)) * mb;
        const std::string tile_id = "stress_tile_" + std::to_string(i + 1);
        workload.jobs.push_back({id, tile_id, static_cast<int>(i % 5) + 1,
            2.0 + static_cast<double>(i % 13), bytes, JobStatus::Pending});
        const std::uint32_t x = static_cast<std::uint32_t>(i % 4096);
        const std::uint32_t y = static_cast<std::uint32_t>((i / 4096) % 4096);
        workload.tiles.push_back({tile_id, 12, x, y, {-122.50, 37.70, -122.40, 37.80},
                                  4.78, bytes, "sha256:stress"});
    }
    return workload;
}

} // namespace project47
