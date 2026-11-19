#include "project47/algorithms/activity_selection.hpp"
#include "project47/algorithms/celebrity.hpp"
#include "project47/algorithms/divide_and_conquer.hpp"
#include "project47/algorithms/matrix_multiplication.hpp"
#include "project47/algorithms/travelling_salesperson.hpp"
#include "project47/algorithms/backtracking.hpp"
#include "project47/algorithms/branch_and_bound.hpp"
#include "project47/algorithms/chinese_remainder.hpp"
#include "project47/algorithms/string_matching.hpp"
#include "project47/algorithms/binary_search.hpp"
#include "project47/algorithms/huffman_coding.hpp"
#include "project47/algorithms/interval_partitioning.hpp"
#include "project47/algorithms/knapsack.hpp"
#include "project47/algorithms/lcs.hpp"
#include "project47/algorithms/matrix_chain.hpp"
#include "project47/algorithms/optimal_bst.hpp"
#include "project47/algorithms/job_order.hpp"
#include "project47/algorithms/quick_sort.hpp"
#include "project47/graphs/bellman_ford.hpp"
#include "project47/graphs/dependency_graph.hpp"
#include "project47/graphs/floyd_warshall.hpp"
#include "project47/graphs/flow_network.hpp"
#include "project47/graphs/bipartite_matching.hpp"
#include "project47/graphs/minimum_spanning_tree.hpp"
#include "project47/graphs/shortest_paths.hpp"
#include "project47/graphs/weighted_graph.hpp"
#include "project47/graphs/weighted_digraph.hpp"
#include "project47/models/compute_node.hpp"
#include "project47/models/storage_node.hpp"
#include "project47/models/tile.hpp"
#include "project47/scheduler/report_export.hpp"
#include "project47/scheduler/scheduler.hpp"
#include "project47/scheduler/workload_io.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string scenario_option = "all";
    std::filesystem::path output_directory = "outputs";
    std::string jobs_csv_path;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help" || argument == "-h") {
            std::cout << "Project 47 CLI\n"
                      << "Usage: project47_cli [--scenario all|normal|capacity-shock|stress]\n"
                      << "                       [--output-dir DIRECTORY] [--jobs FILE.csv]\n"
                      << "CSV columns: id,tile_id,priority,estimated_seconds,input_bytes,prerequisites\n";
            return 0;
        } else if (argument == "--scenario" && i + 1 < argc) {
            scenario_option = argv[++i];
        } else if (argument == "--output-dir" && i + 1 < argc) {
            output_directory = argv[++i];
        } else if (argument == "--jobs" && i + 1 < argc) {
            jobs_csv_path = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete argument: " << argument << "\nUse --help for usage.\n";
            return 2;
        }
    }
    if (!jobs_csv_path.empty() && scenario_option == "all") scenario_option = "normal";
    if (!jobs_csv_path.empty() && scenario_option != "normal") {
        std::cerr << "Custom --jobs input can only be combined with --scenario normal.\n";
        return 2;
    }
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

    std::vector<int> tile_ids{412, 105, 307, 208, 105, 519, 301};
    project47::quick_sort(tile_ids);
    std::cout << "\nTile IDs sorted with Quick Sort: ";
    for (const int id : tile_ids) std::cout << id << ' ';
    std::cout << '\n';

    const auto found = project47::binary_search_iterative(tile_ids, 307);
    std::cout << "Iterative binary search for tile 307: ";
    if (found) std::cout << "found at index " << *found << '\n';
    else std::cout << "not found\n";

    const auto missing = project47::binary_search_recursive(tile_ids, 999);
    std::cout << "Recursive binary search for tile 999: "
              << (missing ? "found" : "not found") << '\n';

    const project47::Tile sample_tile{
        "tile_z12_x1205_y1530", 12, 1205, 1530,
        {-122.45, 37.70, -122.40, 37.75}, 4.78, 2'400'000, "sha256:sample"
    };
    const project47::ComputeNode compute{
        "compute-01", "worker-01.local", 16, 64ULL * 1024 * 1024 * 1024,
        48ULL * 1024 * 1024 * 1024, 850.0, project47::ComputeNodeStatus::Online
    };
    const project47::StorageNode storage{
        "storage-01", "region-west", 10ULL * 1024 * 1024 * 1024 * 1024,
        4ULL * 1024 * 1024 * 1024 * 1024, 1200.0, true
    };

    try {
        sample_tile.validate();
        compute.validate();
        storage.validate();
    } catch (const std::exception& error) {
        std::cerr << "Invalid infrastructure sample: " << error.what() << '\n';
        return 1;
    }

    std::cout << "\nGeospatial records validated\n";
    std::cout << "Tile: " << sample_tile.id << " | zoom " << static_cast<int>(sample_tile.zoom_level)
              << " | " << sample_tile.byte_size << " bytes\n";
    std::cout << "Compute: " << compute.id << " | " << compute.cpu_cores << " CPU cores | "
              << compute.processing_capacity_gflops << " GFLOPS\n";
    std::cout << "Storage: " << storage.id << " | " << storage.available_bytes()
              << " bytes available\n";

    project47::DependencyGraph dependencies;
    for (const auto id : {101ULL, 102ULL, 103ULL, 104ULL}) dependencies.add_job(id);
    dependencies.add_dependency(101, 103);
    dependencies.add_dependency(102, 103);
    dependencies.add_dependency(103, 104);
    const auto order = dependencies.topological_order();
    std::cout << "Dependency-safe job order: ";
    for (const auto id : order) std::cout << id << ' ';
    std::cout << '\n';

    const std::vector<project47::Activity> windows{
        {201, 0.0, 3.0}, {202, 1.0, 4.0}, {203, 3.0, 5.0},
        {204, 4.0, 7.0}, {205, 7.0, 9.0}
    };
    const auto selected = project47::activity_selection(windows);
    const auto partition = project47::interval_partitioning(windows);
    std::cout << "\nGreedy activity selection: " << selected.size()
              << " compatible processing windows selected\n";
    std::cout << "Interval partitioning: " << partition.resource_count
              << " compute slots required for " << partition.assignments.size() << " windows\n";

    const auto codebook = project47::build_huffman_codebook({
        {'-', 1}, {'a', 1}, {'d', 1}, {'e', 1}, {'i', 1}, {'l', 1}, {'t', 1}
    });
    const std::string metadata = "tile-data";
    const auto encoded = codebook.encode(metadata);
    std::cout << "Huffman coding: " << metadata.size() << " metadata symbols -> "
              << encoded.size() << " bits (round-trip "
              << (codebook.decode(encoded) == metadata ? "verified" : "FAILED") << ")\n";

    project47::WeightedGraph network;
    for (const auto& node : {"storage-west", "compute-a", "compute-b", "storage-east"}) {
        network.add_vertex(node);
    }
    network.add_edge("storage-west", "compute-a", 5.0);
    network.add_edge("storage-west", "compute-b", 2.0);
    network.add_edge("compute-a", "compute-b", 1.0);
    network.add_edge("compute-a", "storage-east", 8.0);
    network.add_edge("compute-b", "storage-east", 2.0);

    const auto route = project47::dijkstra(network, "storage-west");
    const auto mst_prim = project47::prim_minimum_spanning_tree(network);
    const auto mst_kruskal = project47::kruskal_minimum_spanning_tree(network);
    std::cout << "Dijkstra route storage-west -> storage-east: ";
    const auto route_nodes = route.path_to("storage-east");
    for (std::size_t i = 0; i < route_nodes.size(); ++i) {
        if (i != 0) std::cout << " -> ";
        std::cout << route_nodes[i];
    }
    std::cout << " | cost " << route.distance.at("storage-east") << "\n";
    std::cout << "Minimum network cost: Prim " << mst_prim.total_weight
              << " | Kruskal " << mst_kruskal.total_weight << "\n";

    const std::vector<project47::KnapsackItem> tile_options{
        {"tile-a", 10, 60}, {"tile-b", 20, 100}, {"tile-c", 30, 120}
    };
    const auto allocation = project47::zero_one_knapsack(tile_options, 50);
    std::cout << "\n0/1 Knapsack: capacity 50 MB | value " << allocation.total_value
              << " | selected ";
    for (const auto index : allocation.selected_indices) {
        std::cout << tile_options[index].id << ' ';
    }
    std::cout << "\n";

    const auto matrix_chain = project47::matrix_chain_order({10, 30, 5, 60});
    std::cout << "Matrix chain: " << matrix_chain.parenthesization
              << " | " << matrix_chain.minimum_multiplications << " scalar multiplications\n";

    const auto lcs = project47::longest_common_subsequence("AGGTAB", "GXTXAYB");
    std::cout << "Metadata LCS: " << lcs.subsequence << " | length " << lcs.length << "\n";

    const auto optimal_bst = project47::optimal_binary_search_tree(
        {"tile-alpha", "tile-beta"}, {3, 3}, {1, 1, 1});
    std::cout << "Optimal metadata index: expected search cost "
              << optimal_bst.expected_cost << "\n";

    project47::DirectedWeightedGraph signed_costs;
    for (const auto& node : {"storage-a", "compute-a", "compute-b", "archive"}) {
        signed_costs.add_vertex(node);
    }
    signed_costs.add_edge("storage-a", "compute-a", 4.0);
    signed_costs.add_edge("storage-a", "compute-b", 5.0);
    signed_costs.add_edge("compute-a", "compute-b", -1.0);
    signed_costs.add_edge("compute-a", "archive", 6.0);
    signed_costs.add_edge("compute-b", "archive", 2.0);
    const auto bellman = project47::bellman_ford(signed_costs, "storage-a");
    const auto floyd = project47::floyd_warshall(signed_costs);
    std::cout << "Bellman-Ford route storage-a -> archive: ";
    const auto signed_path = bellman.path_to("archive");
    for (std::size_t i = 0; i < signed_path.size(); ++i) {
        if (i != 0) std::cout << " -> ";
        std::cout << signed_path[i];
    }
    std::cout << " | cost " << bellman.distance.at("archive") << "\n";
    std::cout << "Floyd-Warshall cross-check: storage-a -> archive | cost "
              << floyd.distance_between("storage-a", "archive")
              << " | negative cycle: " << (floyd.has_negative_cycle ? "yes" : "no") << "\n";

    const auto queens = project47::solve_n_queens(8);
    const auto subset = project47::subset_sum_backtracking({12, 7, 11, 5, 12}, 23);
    std::cout << "\nBacktracking: N-Queens(8) = " << queens.solutions.size()
              << " solutions | " << queens.nodes_explored << " search nodes\n";
    std::cout << "Subset sum: 23 MB metadata bundle -> " << subset.selected_indices.size()
              << " exact subsets | " << subset.nodes_explored << " search nodes\n";

    const std::vector<project47::BnBItem> candidate_tiles{
        {"tile-a", 10, 60}, {"tile-b", 20, 100},
        {"tile-c", 30, 120}, {"tile-d", 15, 75}
    };
    constexpr std::size_t memory_budget_mb = 50;
    std::cout << "Branch-and-bound tile allocation (capacity " << memory_budget_mb << " MB):\n";
    for (const auto strategy : {project47::BranchAndBoundStrategy::FIFO,
                                project47::BranchAndBoundStrategy::LIFO,
                                project47::BranchAndBoundStrategy::LeastCost}) {
        const auto decision = project47::branch_and_bound_knapsack(
            candidate_tiles, memory_budget_mb, strategy);
        std::cout << "  " << project47::to_string(strategy)
                  << ": value " << decision.total_value
                  << ", weight " << decision.total_weight << " MB, expanded "
                  << decision.nodes_expanded << " nodes, selected";
        for (const auto index : decision.selected_indices) {
            std::cout << ' ' << candidate_tiles[index].id;
        }
        std::cout << "\n";
    }



    project47::FlowNetwork flow_network;
    for (const auto& vertex : {"storage-west", "compute-a", "compute-b", "archive"}) {
        flow_network.add_vertex(vertex);
    }
    flow_network.add_edge("storage-west", "compute-a", 10);
    flow_network.add_edge("storage-west", "compute-b", 5);
    flow_network.add_edge("compute-a", "compute-b", 15);
    flow_network.add_edge("compute-a", "archive", 10);
    flow_network.add_edge("compute-b", "archive", 10);
    const auto flow = flow_network.ford_fulkerson_max_flow("storage-west", "archive");
    std::cout << "\nFord-Fulkerson tile-transfer capacity: " << flow.maximum_flow
              << " MB/unit | " << flow.augmentations << " augmenting paths\n";

    project47::BipartiteGraph assignments;
    for (const auto& tile : {"tile-12-05", "tile-12-06", "tile-12-07"}) {
        assignments.add_left_vertex(tile);
    }
    for (const auto& worker : {"compute-01", "compute-02", "compute-03"}) {
        assignments.add_right_vertex(worker);
    }
    assignments.add_edge("tile-12-05", "compute-01");
    assignments.add_edge("tile-12-05", "compute-02");
    assignments.add_edge("tile-12-06", "compute-01");
    assignments.add_edge("tile-12-07", "compute-02");
    assignments.add_edge("tile-12-07", "compute-03");
    const auto matching = assignments.maximum_matching();
    std::cout << "Maximum bipartite tile/worker matching: " << matching.cardinality() << " assignments\n";
    for (const auto& pair : matching.pairs) {
        std::cout << "  " << pair.left << " -> " << pair.right << "\n";
    }

    const auto crt = project47::chinese_remainder_theorem({{2, 3}, {3, 5}, {2, 7}});
    std::cout << "Chinese Remainder utility: t = " << crt.residue
              << " (mod " << crt.modulus << ")\n";

    const std::string event_log = "tile_12_05 uploaded; tile_12_07 uploaded; tile_12_05 indexed";
    const std::string query = "tile_12_05";
    const auto naive_matches = project47::naive_string_search(event_log, query);
    const auto rabin_matches = project47::rabin_karp_search(event_log, query);
    const auto automaton_matches = project47::finite_automaton_search(event_log, query);
    const auto kmp_matches = project47::kmp_search(event_log, query);
    std::cout << "Metadata pattern search '" << query << "': naive " << naive_matches.size()
              << ", Rabin-Karp " << rabin_matches.size()
              << ", finite automaton " << automaton_matches.size()
              << ", KMP " << kmp_matches.size() << " matches\n";

    const auto max_subarray = project47::maximum_subarray_divide_conquer(
        {-2, 1, -3, 4, -1, 2, 1, -5, 4});
    std::cout << "Maximum subarray: sum " << max_subarray.sum
              << " | range [" << max_subarray.begin << ", " << max_subarray.end << ")\n";
    std::cout << "Fast powering: 2^20 = " << project47::fast_power(2.0, 20) << "\n";

    const std::vector<std::vector<bool>> knows{
        {false, true, true}, {false, false, true}, {false, false, false}
    };
    const auto celebrity = project47::find_celebrity_divide_conquer(knows);
    std::cout << "Celebrity discovery (divide-and-conquer): "
              << (celebrity ? std::to_string(*celebrity) : std::string("none")) << "\n";

    project47::IntMatrix matrix_a(64, std::vector<std::int64_t>(64, 1));
    project47::IntMatrix matrix_b(64, std::vector<std::int64_t>(64, 2));
    const auto classical_product = project47::classical_matrix_multiply(matrix_a, matrix_b);
    const auto strassen_product = project47::strassen_matrix_multiply(matrix_a, matrix_b);
    std::cout << "Matrix multiplication (64x64): classical vs Strassen = "
              << (classical_product == strassen_product ? "identical" : "MISMATCH")
              << " | C[0][0]=" << strassen_product[0][0] << "\n";

    const double tsp_inf = std::numeric_limits<double>::infinity();
    const auto tsp = project47::travelling_salesperson_held_karp({
        {tsp_inf, 10, 15, 20}, {10, tsp_inf, 35, 25},
        {15, 35, tsp_inf, 30}, {20, 25, 30, tsp_inf}});
    std::cout << "Bounded TSP (Held-Karp): minimum tour cost " << tsp.minimum_cost << " | tour ";
    for (std::size_t i = 0; i < tsp.tour.size(); ++i) {
        if (i != 0) std::cout << " -> ";
        std::cout << tsp.tour[i];
    }
    std::cout << "\n";

    std::vector<project47::Scenario> scenarios;
    if (scenario_option == "all") {
        scenarios = {project47::Scenario::Normal, project47::Scenario::CapacityShock,
                     project47::Scenario::Stress};
    } else if (scenario_option == "normal") {
        scenarios = {project47::Scenario::Normal};
    } else if (scenario_option == "capacity-shock") {
        scenarios = {project47::Scenario::CapacityShock};
    } else if (scenario_option == "stress") {
        scenarios = {project47::Scenario::Stress};
    } else {
        std::cerr << "Invalid scenario: " << scenario_option << "\n";
        return 2;
    }

    std::cout << "\n=== Integrated satellite workload scheduler ===\n";
    try {
        for (const project47::Scenario scenario : scenarios) {
            project47::SchedulerWorkload workload =
                scenario == project47::Scenario::Stress
                    ? project47::make_stress_workload(128)
                    : project47::make_demo_workload(scenario);

            if (!jobs_csv_path.empty()) {
                std::ifstream jobs_file(jobs_csv_path);
                if (!jobs_file) throw std::runtime_error("Cannot open jobs CSV: " + jobs_csv_path);
                const auto loaded = project47::load_jobs_csv(jobs_file);
                workload.jobs = loaded.jobs;
                workload.dependencies = loaded.dependencies;
                workload.tiles.clear();
                std::map<std::string, std::uint64_t> tile_sizes;
                for (const auto& job : workload.jobs) {
                    const auto [it, inserted] = tile_sizes.emplace(job.tile_id, job.input_bytes);
                    if (!inserted && it->second != job.input_bytes) {
                        throw std::invalid_argument("Jobs referencing the same tile must use identical input_bytes");
                    }
                }
                std::uint32_t coordinate = 0;
                for (const auto& [tile_id, bytes] : tile_sizes) {
                    const std::uint32_t x = coordinate % 4096U;
                    const std::uint32_t y = (coordinate / 4096U) % 4096U;
                    ++coordinate;
                    workload.tiles.push_back({tile_id, 12, x, y,
                        {-122.50, 37.70, -122.40, 37.80}, 4.78, bytes, "sha256:csv-input"});
                }
            }

            const auto result = project47::run_scheduler(workload, scenario);
            project47::export_scheduler_reports(result, output_directory);
            std::cout << "Scenario: " << result.scenario << "\n"
                      << "  Jobs completed: " << result.completed_count() << "/" << result.total_jobs << "\n"
                      << "  Jobs deferred: " << result.deferred_count() << "\n"
                      << "  Scheduling waves: " << result.waves << "\n"
                      << "  Maximum-matching assignments: " << result.matching_assignments << "\n"
                      << "  Processing work: " << result.total_estimated_processing_seconds << " s\n"
                      << "  Estimated wave-barrier makespan: " << result.estimated_makespan_seconds << " s\n"
                      << "  Aggregate route cost: " << result.total_route_cost << "\n"
                      << "  Reports: " << (output_directory / (result.scenario + "_jobs.csv")).string()
                      << " and " << (output_directory / (result.scenario + "_report.json")).string() << "\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Scheduler run failed: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
