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
#include "project47/algorithms/merge_sort.hpp"
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

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int checks_run = 0;

void check(bool condition, const char* expression, const char* file, int line) {
    ++checks_run;
    if (!condition) {
        throw std::runtime_error(std::string(file) + ":" + std::to_string(line)
                                 + ": check failed: " + expression);
    }
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)

template <typename Exception, typename Function>
void check_throws(Function&& function) {
    bool threw = false;
    try {
        function();
    } catch (const Exception&) {
        threw = true;
    }
    CHECK(threw);
}

struct TaggedValue { int key; std::string tag; };

void test_merge_sort_integers() {
    std::vector<int> values{7, -2, 4, 4, 0, 19, 1};
    project47::merge_sort(values);
    CHECK((values == std::vector<int>{-2, 0, 1, 4, 4, 7, 19}));
}

void test_merge_sort_empty_single_and_stability() {
    std::vector<int> empty;
    std::vector<int> single{42};
    project47::merge_sort(empty);
    project47::merge_sort(single);
    CHECK(empty.empty());
    CHECK((single == std::vector<int>{42}));

    std::vector<TaggedValue> tagged{{2, "first"}, {1, "middle"}, {2, "second"}};
    project47::merge_sort(tagged, [](const TaggedValue& a, const TaggedValue& b) {
        return a.key < b.key;
    });
    CHECK(tagged[0].tag == "middle");
    CHECK(tagged[1].tag == "first");
    CHECK(tagged[2].tag == "second");
}

void test_job_scheduling_order() {
    std::vector<project47::Job> jobs{
        {3, "t3", 2, 5.0, 100, project47::JobStatus::Pending},
        {2, "t2", 4, 8.0, 100, project47::JobStatus::Pending},
        {1, "t1", 4, 3.0, 100, project47::JobStatus::Pending},
        {4, "t4", 4, 3.0, 100, project47::JobStatus::Pending}
    };
    project47::sort_jobs_for_scheduling(jobs);
    CHECK(jobs[0].id == 1);
    CHECK(jobs[1].id == 4);
    CHECK(jobs[2].id == 2);
    CHECK(jobs[3].id == 3);
}

void test_invalid_job_is_rejected() {
    std::vector<project47::Job> jobs{
        {0, "tile", 1, 2.0, 100, project47::JobStatus::Pending}
    };
    check_throws<std::invalid_argument>([&] { project47::sort_jobs_for_scheduling(jobs); });
    project47::Job nan_job{1, "tile", 1, std::numeric_limits<double>::quiet_NaN(), 100,
                           project47::JobStatus::Pending};
    check_throws<std::invalid_argument>([&] { nan_job.validate(); });
}

void test_quick_sort_basic_and_duplicates() {
    std::vector<int> values{9, 3, 7, 3, -1, 9, 0, 3};
    project47::quick_sort(values);
    CHECK((values == std::vector<int>{-1, 0, 3, 3, 3, 7, 9, 9}));
}

void test_quick_sort_edge_cases() {
    std::vector<int> empty;
    std::vector<int> single{5};
    std::vector<int> equal(128, 11);
    std::vector<int> sorted(1000);
    for (int i = 0; i < 1000; ++i) sorted[static_cast<std::size_t>(i)] = i;
    project47::quick_sort(empty);
    project47::quick_sort(single);
    project47::quick_sort(equal);
    project47::quick_sort(sorted);
    CHECK(empty.empty());
    CHECK((single == std::vector<int>{5}));
    CHECK(std::all_of(equal.begin(), equal.end(), [](int x) { return x == 11; }));
    CHECK(std::is_sorted(sorted.begin(), sorted.end()));

    std::vector<int> descending{1, 5, 2, 9, 0};
    project47::quick_sort(descending, std::greater<int>{});
    CHECK((descending == std::vector<int>{9, 5, 2, 1, 0}));

    std::mt19937 generator(47);
    std::uniform_int_distribution<int> distribution(-500, 500);
    for (std::size_t size = 0; size < 256; ++size) {
        std::vector<int> actual(size);
        for (int& value : actual) value = distribution(generator);
        std::vector<int> expected = actual;
        std::sort(expected.begin(), expected.end());
        project47::quick_sort(actual);
        CHECK(actual == expected);
    }
}

void test_binary_search_variants() {
    const std::vector<int> values{-5, 0, 2, 2, 2, 8, 14};
    CHECK(project47::binary_search_iterative(values, -5) == 0);
    CHECK(project47::binary_search_iterative(values, 2) == 2);
    CHECK(project47::binary_search_iterative(values, 14) == 6);
    CHECK(!project47::binary_search_iterative(values, 7).has_value());
    CHECK(!project47::binary_search_iterative(std::vector<int>{}, 7).has_value());
    CHECK(project47::binary_search_recursive(values, -5) == 0);
    CHECK(project47::binary_search_recursive(values, 2) == 2);
    CHECK(project47::binary_search_recursive(values, 14) == 6);
    CHECK(!project47::binary_search_recursive(values, 7).has_value());
    CHECK(!project47::binary_search_recursive(std::vector<int>{}, 7).has_value());

    const std::vector<int> descending{20, 15, 10, 5, 0};
    CHECK(project47::binary_search_iterative(descending, 10, std::greater<int>{}) == 2);
    CHECK(project47::binary_search_recursive(descending, 5, std::greater<int>{}) == 3);
    CHECK(!project47::binary_search_recursive(descending, 7, std::greater<int>{}).has_value());
}

project47::Tile valid_tile() {
    return {"tile_z12_x1205_y1530", 12, 1205, 1530,
            {-122.45, 37.70, -122.40, 37.75}, 4.78, 2'400'000, "sha256:test"};
}

void test_tile_validation() {
    const auto tile = valid_tile();
    tile.validate();
    CHECK(tile.id == "tile_z12_x1205_y1530");
    CHECK(tile.zoom_level == 12);
    CHECK(tile.byte_size == 2'400'000);

    auto invalid = tile;
    invalid.id.clear();
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = tile;
    invalid.column = 4096; // Invalid at zoom 12: valid range is [0, 4096).
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = tile;
    invalid.bounds.max_longitude = 181.0;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = tile;
    invalid.ground_resolution_m = 0.0;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = tile;
    invalid.byte_size = 0;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
}

void test_compute_node_validation() {
    project47::ComputeNode node{"compute-01", "worker-01.local", 16,
        64ULL * 1024 * 1024 * 1024, 48ULL * 1024 * 1024 * 1024,
        850.0, project47::ComputeNodeStatus::Online};
    node.validate();
    auto invalid = node;
    invalid.available_memory_bytes = invalid.memory_bytes + 1;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = node;
    invalid.cpu_cores = 0;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = node;
    invalid.processing_capacity_gflops = std::numeric_limits<double>::infinity();
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
}

void test_storage_node_validation_and_capacity() {
    project47::StorageNode storage{"storage-01", "region-west", 1000, 400, 1200.0, true};
    storage.validate();
    CHECK(storage.available_bytes() == 600);
    auto invalid = storage;
    invalid.used_bytes = invalid.capacity_bytes + 1;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
    invalid = storage;
    invalid.bandwidth_mbps = -1.0;
    check_throws<std::invalid_argument>([&] { invalid.validate(); });
}

void test_dependency_graph_order_and_duplicates() {
    project47::DependencyGraph graph;
    for (const auto id : {40ULL, 10ULL, 30ULL, 20ULL}) graph.add_job(id);
    CHECK(graph.job_count() == 4);
    CHECK(graph.add_dependency(10, 30));
    CHECK(!graph.add_dependency(10, 30));
    CHECK(graph.add_dependency(20, 30));
    CHECK(graph.add_dependency(30, 40));
    CHECK(graph.dependency_count() == 3);

    const auto order = graph.topological_order();
    CHECK((order == std::vector<std::uint64_t>{10, 20, 30, 40}));
    CHECK(!graph.has_cycle());
}

void test_dependency_graph_rejects_invalid_edges_and_cycles() {
    project47::DependencyGraph graph;
    graph.add_job(1);
    graph.add_job(2);
    check_throws<std::invalid_argument>([&] { graph.add_dependency(1, 3); });
    check_throws<std::invalid_argument>([&] { graph.add_dependency(1, 1); });
    check_throws<std::invalid_argument>([&] { graph.add_dependency(0, 2); });

    graph.add_dependency(1, 2);
    graph.add_dependency(2, 1);
    CHECK(graph.has_cycle());
    check_throws<std::logic_error>([&] { (void)graph.topological_order(); });
}


void test_activity_selection_greedy() {
    std::vector<project47::Activity> activities{
        {1, 1.0, 4.0}, {2, 3.0, 5.0}, {3, 0.0, 6.0},
        {4, 5.0, 7.0}, {5, 8.0, 9.0}, {6, 5.0, 9.0}
    };
    const auto selected = project47::activity_selection(activities);
    CHECK(selected.size() == 3);
    CHECK(selected[0].id == 1);
    CHECK(selected[1].id == 4);
    CHECK(selected[2].id == 5);
    CHECK(selected[1].start_time >= selected[0].finish_time);

    CHECK(project47::activity_selection({}).empty());
    check_throws<std::invalid_argument>([] {
        (void)project47::activity_selection({{1, 2.0, 2.0}});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::activity_selection({{1, 0.0, 1.0}, {1, 2.0, 3.0}});
    });
}

void test_interval_partitioning_minimum_resources() {
    const std::vector<project47::Activity> activities{
        {1, 0.0, 4.0}, {2, 1.0, 3.0}, {3, 3.0, 5.0},
        {4, 4.0, 6.0}, {5, 6.0, 7.0}
    };
    const auto result = project47::interval_partitioning(activities);
    CHECK(result.resource_count == 2);
    CHECK(result.assignments.size() == activities.size());
    for (std::size_t i = 0; i < result.assignments.size(); ++i) {
        for (std::size_t j = i + 1; j < result.assignments.size(); ++j) {
            const auto& a = result.assignments[i];
            const auto& b = result.assignments[j];
            if (a.resource_id == b.resource_id) {
                CHECK(a.activity.finish_time <= b.activity.start_time ||
                      b.activity.finish_time <= a.activity.start_time);
            }
        }
    }
    CHECK(project47::interval_partitioning({}).resource_count == 0);
    const auto one = project47::interval_partitioning({{8, 2.0, 5.0}});
    CHECK(one.resource_count == 1);
    CHECK(one.assignments[0].resource_id == 1);
}

void test_huffman_encoding_and_validation() {
    const auto codebook = project47::build_huffman_codebook({
        {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}
    });
    const std::string input = "facefeed";
    const auto encoded = codebook.encode(input);
    CHECK(!encoded.empty());
    CHECK(codebook.decode(encoded) == input);
    CHECK(codebook.codes().size() == 6);
    for (const auto& [symbol_a, code_a] : codebook.codes()) {
        for (const auto& [symbol_b, code_b] : codebook.codes()) {
            if (symbol_a != symbol_b) {
                CHECK(code_a.size() <= code_b.size() ?
                      code_b.compare(0, code_a.size(), code_a) != 0 :
                      code_a.compare(0, code_b.size(), code_b) != 0);
            }
        }
    }

    const auto single = project47::build_huffman_codebook({{'x', 10}});
    CHECK(single.codes().at('x') == "0");
    CHECK(single.encode("xxx") == "000");
    CHECK(single.decode("000") == "xxx");
    CHECK(project47::build_huffman_codebook({}).codes().empty());
    CHECK(project47::build_huffman_codebook({}).encode("").empty());
    CHECK(project47::build_huffman_codebook({}).decode("").empty());
    check_throws<std::invalid_argument>([&] { (void)codebook.encode("z"); });
    check_throws<std::invalid_argument>([&] { (void)codebook.decode("0102"); });
    check_throws<std::invalid_argument>([] {
        (void)project47::build_huffman_codebook({{'x', 0}});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::build_huffman_codebook({{'a', 1}, {'b', 2}, {'c', 4}}).decode("0");
    });
}

void test_weighted_graph_validation_and_dijkstra() {
    project47::WeightedGraph graph;
    for (const auto& vertex : {"storage-west", "compute-a", "compute-b", "storage-east", "isolated"}) {
        CHECK(graph.add_vertex(vertex));
    }
    CHECK(!graph.add_vertex("compute-a"));
    graph.add_edge("storage-west", "compute-a", 5.0);
    graph.add_edge("storage-west", "compute-b", 2.0);
    graph.add_edge("compute-a", "compute-b", 1.0);
    graph.add_edge("compute-a", "storage-east", 8.0);
    graph.add_edge("compute-b", "storage-east", 2.0);
    CHECK(graph.vertex_count() == 5);
    check_throws<std::invalid_argument>([&] { graph.add_edge("missing", "compute-a", 1.0); });
    check_throws<std::invalid_argument>([&] { graph.add_edge("compute-a", "compute-a", 1.0); });
    check_throws<std::invalid_argument>([&] { graph.add_edge("compute-a", "compute-b", -1.0); });
    check_throws<std::invalid_argument>([&] {
        graph.add_edge("compute-a", "compute-b", std::numeric_limits<double>::quiet_NaN());
    });

    const auto paths = project47::dijkstra(graph, "storage-west");
    CHECK(paths.distance.at("storage-west") == 0.0);
    CHECK(paths.distance.at("compute-b") == 2.0);
    CHECK(paths.distance.at("compute-a") == 3.0);
    CHECK(paths.distance.at("storage-east") == 4.0);
    CHECK(paths.path_to("storage-east") ==
          (std::vector<std::string>{"storage-west", "compute-b", "storage-east"}));
    CHECK(paths.path_to("isolated").empty());
    CHECK(paths.path_to("storage-west") == (std::vector<std::string>{"storage-west"}));
    check_throws<std::invalid_argument>([&] { (void)paths.path_to("unknown"); });
    check_throws<std::invalid_argument>([&] { (void)project47::dijkstra(graph, "unknown"); });
}

void test_prim_and_kruskal_minimum_spanning_tree() {
    project47::WeightedGraph graph;
    for (const auto& vertex : {"A", "B", "C", "D"}) graph.add_vertex(vertex);
    graph.add_edge("A", "B", 5.0);
    graph.add_edge("A", "C", 2.0);
    graph.add_edge("B", "C", 1.0);
    graph.add_edge("B", "D", 2.0);
    graph.add_edge("C", "D", 8.0);
    graph.add_edge("A", "D", 20.0);

    const auto prim = project47::prim_minimum_spanning_tree(graph, "A");
    const auto kruskal = project47::kruskal_minimum_spanning_tree(graph);
    CHECK(prim.edges.size() == 3);
    CHECK(kruskal.edges.size() == 3);
    CHECK(prim.total_weight == 5.0);
    CHECK(kruskal.total_weight == 5.0);
    CHECK(prim.total_weight == kruskal.total_weight);
    check_throws<std::invalid_argument>([&] {
        (void)project47::prim_minimum_spanning_tree(graph, "missing");
    });

    project47::WeightedGraph disconnected;
    disconnected.add_vertex("one");
    disconnected.add_vertex("two");
    disconnected.add_vertex("three");
    disconnected.add_edge("one", "two", 1.0);
    check_throws<std::logic_error>([&] {
        (void)project47::prim_minimum_spanning_tree(disconnected);
    });
    check_throws<std::logic_error>([&] {
        (void)project47::kruskal_minimum_spanning_tree(disconnected);
    });

    project47::WeightedGraph singleton;
    singleton.add_vertex("only");
    CHECK(project47::prim_minimum_spanning_tree(singleton).edges.empty());
    CHECK(project47::kruskal_minimum_spanning_tree(singleton).edges.empty());
}


void test_zero_one_knapsack() {
    const std::vector<project47::KnapsackItem> items{
        {"tile-a", 10, 60}, {"tile-b", 20, 100}, {"tile-c", 30, 120}
    };
    const auto result = project47::zero_one_knapsack(items, 50);
    CHECK(result.total_value == 220);
    CHECK(result.total_weight == 50);
    CHECK((result.selected_indices == std::vector<std::size_t>{1, 2}));

    const auto with_zero_weight = project47::zero_one_knapsack(
        {{"metadata", 0, 7}, {"oversized", 9, 100}}, 0);
    CHECK(with_zero_weight.total_value == 7);
    CHECK(with_zero_weight.total_weight == 0);
    CHECK((with_zero_weight.selected_indices == std::vector<std::size_t>{0}));
    CHECK(project47::zero_one_knapsack({}, 5).total_value == 0);
    CHECK(project47::zero_one_knapsack({{"zero", 0, 0}}, 0).selected_indices.empty());

    check_throws<std::invalid_argument>([] {
        (void)project47::zero_one_knapsack({{"", 1, 1}}, 1);
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::zero_one_knapsack({{"same", 1, 1}, {"same", 2, 2}}, 3);
    });
    check_throws<std::length_error>([] {
        (void)project47::zero_one_knapsack({}, std::numeric_limits<std::size_t>::max());
    });
}

void test_matrix_chain_multiplication() {
    const auto result = project47::matrix_chain_order({30, 35, 15, 5, 10, 20, 25});
    CHECK(result.minimum_multiplications == 15125);
    CHECK(result.parenthesization.find("A1") != std::string::npos);
    CHECK(result.parenthesization.find("A6") != std::string::npos);
    CHECK(result.costs[1][6] == 15125);
    CHECK(project47::matrix_chain_order({10, 20}).minimum_multiplications == 0);
    CHECK(project47::matrix_chain_order({10, 20}).parenthesization == "A1");

    check_throws<std::invalid_argument>([] { (void)project47::matrix_chain_order({}); });
    check_throws<std::invalid_argument>([] { (void)project47::matrix_chain_order({4, 0, 8}); });
    check_throws<std::overflow_error>([] {
        const auto maximum = std::numeric_limits<std::size_t>::max();
        (void)project47::matrix_chain_order({maximum, maximum, maximum});
    });
}

void test_longest_common_subsequence() {
    const auto result = project47::longest_common_subsequence("AGGTAB", "GXTXAYB");
    CHECK(result.length == 4);
    CHECK(result.subsequence == "GTAB");
    CHECK(project47::longest_common_subsequence("", "abc").length == 0);
    CHECK(project47::longest_common_subsequence("abc", "").subsequence.empty());
    CHECK(project47::longest_common_subsequence("same", "same").subsequence == "same");

    const auto repeated = project47::longest_common_subsequence("ABCBDAB", "BDCABA");
    CHECK(repeated.length == 4);
    std::size_t cursor = 0;
    for (const char c : repeated.subsequence) {
        cursor = std::string("ABCBDAB").find(c, cursor);
        CHECK(cursor != std::string::npos);
        ++cursor;
    }
}

void test_optimal_binary_search_tree() {
    const auto result = project47::optimal_binary_search_tree(
        {"alpha", "beta"}, {3, 3}, {1, 1, 1});
    CHECK(result.expected_cost == 17);
    CHECK(result.key_count == 2);
    CHECK(result.root[1][2] == 0 || result.root[1][2] == 1);

    const auto empty = project47::optimal_binary_search_tree({}, {}, {9});
    CHECK(empty.expected_cost == 9);
    CHECK(empty.key_count == 0);
    const auto singleton = project47::optimal_binary_search_tree({"tile"}, {5}, {1, 1});
    CHECK(singleton.expected_cost == 9);
    CHECK(singleton.root[1][1] == 0);

    check_throws<std::invalid_argument>([] {
        (void)project47::optimal_binary_search_tree({"a", "b"}, {1}, {1, 1, 1});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::optimal_binary_search_tree({"b", "a"}, {1, 1}, {1, 1, 1});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::optimal_binary_search_tree({"a", "a"}, {1, 1}, {1, 1, 1});
    });
    check_throws<std::overflow_error>([] {
        (void)project47::optimal_binary_search_tree(
            {"a"}, {std::numeric_limits<std::uint64_t>::max()}, {1, 0});
    });
}

project47::DirectedWeightedGraph make_bellman_ford_sample() {
    project47::DirectedWeightedGraph graph;
    for (const auto& vertex : {"s", "t", "x", "y", "z", "isolated"}) graph.add_vertex(vertex);
    graph.add_edge("s", "t", 6);
    graph.add_edge("s", "y", 7);
    graph.add_edge("t", "x", 5);
    graph.add_edge("t", "y", 8);
    graph.add_edge("t", "z", -4);
    graph.add_edge("x", "t", -2);
    graph.add_edge("y", "x", -3);
    graph.add_edge("y", "z", 9);
    graph.add_edge("z", "s", 2);
    graph.add_edge("z", "x", 7);
    return graph;
}

void test_bellman_ford_negative_edges_and_cycles() {
    const auto graph = make_bellman_ford_sample();
    const auto result = project47::bellman_ford(graph, "s");
    CHECK(!result.has_reachable_negative_cycle);
    CHECK(result.distance.at("s") == 0.0);
    CHECK(result.distance.at("t") == 2.0);
    CHECK(result.distance.at("x") == 4.0);
    CHECK(result.distance.at("y") == 7.0);
    CHECK(result.distance.at("z") == -2.0);
    CHECK(std::isinf(result.distance.at("isolated")));
    CHECK(result.path_to("z") == (std::vector<std::string>{"s", "y", "x", "t", "z"}));
    CHECK(result.path_to("isolated").empty());
    check_throws<std::invalid_argument>([&] { (void)result.path_to("missing"); });
    check_throws<std::invalid_argument>([&] { (void)project47::bellman_ford(graph, "missing"); });

    project47::DirectedWeightedGraph negative_cycle;
    negative_cycle.add_vertex("source");
    negative_cycle.add_vertex("a");
    negative_cycle.add_vertex("b");
    negative_cycle.add_edge("source", "a", 1);
    negative_cycle.add_edge("a", "b", -2);
    negative_cycle.add_edge("b", "a", -2);
    const auto cycle_result = project47::bellman_ford(negative_cycle, "source");
    CHECK(cycle_result.has_reachable_negative_cycle);
    check_throws<std::logic_error>([&] { (void)cycle_result.path_to("b"); });

    project47::DirectedWeightedGraph unreachable_cycle;
    unreachable_cycle.add_vertex("source");
    unreachable_cycle.add_vertex("a");
    unreachable_cycle.add_vertex("b");
    unreachable_cycle.add_edge("a", "b", -1);
    unreachable_cycle.add_edge("b", "a", -1);
    CHECK(!project47::bellman_ford(unreachable_cycle, "source").has_reachable_negative_cycle);
    check_throws<std::invalid_argument>([&] { unreachable_cycle.add_edge("source", "missing", 1); });
    check_throws<std::invalid_argument>([&] {
        unreachable_cycle.add_edge("source", "source", std::numeric_limits<double>::infinity());
    });
}

void test_floyd_warshall_all_pairs() {
    const auto graph = make_bellman_ford_sample();
    const auto result = project47::floyd_warshall(graph);
    CHECK(!result.has_negative_cycle);
    CHECK(result.distance_between("s", "t") == 2.0);
    CHECK(result.distance_between("s", "z") == -2.0);
    CHECK(result.distance_between("y", "z") == -9.0);
    CHECK(std::isinf(result.distance_between("isolated", "s")));
    CHECK(result.path("s", "z") ==
          (std::vector<std::string>{"s", "y", "x", "t", "z"}));
    CHECK(result.path("s", "isolated").empty());
    CHECK(result.path("s", "s") == (std::vector<std::string>{"s"}));
    check_throws<std::invalid_argument>([&] { (void)result.distance_between("missing", "s"); });
    check_throws<std::invalid_argument>([&] { (void)result.path("s", "missing"); });

    project47::DirectedWeightedGraph negative_cycle;
    negative_cycle.add_vertex("a");
    negative_cycle.add_vertex("b");
    negative_cycle.add_edge("a", "b", -2);
    negative_cycle.add_edge("b", "a", 1);
    const auto cycle_result = project47::floyd_warshall(negative_cycle);
    CHECK(cycle_result.has_negative_cycle);
    check_throws<std::logic_error>([&] { (void)cycle_result.path("a", "b"); });

    project47::DirectedWeightedGraph empty;
    CHECK(project47::floyd_warshall(empty).vertices.empty());
}


void test_n_queens_backtracking() {
    const auto empty = project47::solve_n_queens(0);
    CHECK(empty.solutions.size() == 1);
    CHECK(empty.solutions.front().empty());
    CHECK(!empty.truncated);
    const auto one = project47::solve_n_queens(1);
    CHECK(one.solutions.size() == 1);
    CHECK((one.solutions.front() == std::vector<std::size_t>{0}));

    const auto four = project47::solve_n_queens(4);
    CHECK(four.solutions.size() == 2);
    CHECK(!four.truncated);
    for (const auto& placement : four.solutions) {
        CHECK(placement.size() == 4);
        for (std::size_t row_a = 0; row_a < placement.size(); ++row_a) {
            for (std::size_t row_b = row_a + 1; row_b < placement.size(); ++row_b) {
                CHECK(placement[row_a] != placement[row_b]);
                const auto row_distance = row_b - row_a;
                const auto col_distance = placement[row_a] > placement[row_b]
                    ? placement[row_a] - placement[row_b]
                    : placement[row_b] - placement[row_a];
                CHECK(row_distance != col_distance);
            }
        }
    }
    const auto eight = project47::solve_n_queens(8);
    CHECK(eight.solutions.size() == 92);
    CHECK(!eight.truncated);
    CHECK(eight.nodes_explored > 0);
    const auto limited = project47::solve_n_queens(8, 5);
    CHECK(limited.solutions.size() == 5);
    CHECK(limited.truncated);
    CHECK(limited.nodes_explored < eight.nodes_explored);
    check_throws<std::invalid_argument>([] { (void)project47::solve_n_queens(16); });
}

void test_subset_sum_backtracking() {
    const std::vector<std::int64_t> values{12, 7, 11, 5, 12};
    const auto result = project47::subset_sum_backtracking(values, 23);
    CHECK(result.selected_indices.size() == 3);
    CHECK(!result.truncated);
    CHECK(result.nodes_explored == 63);
    for (const auto& indices : result.selected_indices) {
        std::int64_t sum = 0;
        for (const auto index : indices) {
            CHECK(index < values.size());
            sum += values[index];
        }
        CHECK(sum == 23);
    }
    const auto signed_values = project47::subset_sum_backtracking({-3, 1, 2, 4}, 0);
    CHECK(signed_values.selected_indices.size() == 2);
    bool found_empty_subset = false;
    bool found_signed_combination = false;
    for (const auto& indices : signed_values.selected_indices) {
        found_empty_subset = found_empty_subset || indices.empty();
        found_signed_combination = found_signed_combination ||
            indices == std::vector<std::size_t>{0, 1, 2};
    }
    CHECK(found_empty_subset);
    CHECK(found_signed_combination);
    const auto empty_target = project47::subset_sum_backtracking({}, 0);
    CHECK(empty_target.selected_indices.size() == 1);
    CHECK(empty_target.selected_indices.front().empty());
    const auto limited = project47::subset_sum_backtracking({0, 0, 0}, 0, 1);
    CHECK(limited.selected_indices.size() == 1);
    CHECK(limited.truncated);
    check_throws<std::overflow_error>([] {
        (void)project47::subset_sum_backtracking(
            {std::numeric_limits<std::int64_t>::max(), 1}, 0);
    });
}

void test_branch_and_bound_strategies_match_dynamic_programming() {
    const std::vector<project47::BnBItem> items{
        {"tile-a", 10, 60}, {"tile-b", 20, 100},
        {"tile-c", 30, 120}, {"tile-d", 15, 75}
    };
    const auto fifo = project47::branch_and_bound_knapsack(
        items, 50, project47::BranchAndBoundStrategy::FIFO);
    const auto lifo = project47::branch_and_bound_knapsack(
        items, 50, project47::BranchAndBoundStrategy::LIFO);
    const auto least_cost = project47::branch_and_bound_knapsack(
        items, 50, project47::BranchAndBoundStrategy::LeastCost);
    CHECK(fifo.total_value == 235);
    CHECK(lifo.total_value == fifo.total_value);
    CHECK(least_cost.total_value == fifo.total_value);
    CHECK(fifo.total_weight <= 50);
    CHECK(lifo.total_weight <= 50);
    CHECK(least_cost.total_weight <= 50);
    CHECK(fifo.nodes_expanded > 0);
    CHECK(lifo.nodes_expanded > 0);
    CHECK(least_cost.nodes_expanded > 0);
    CHECK(fifo.nodes_generated >= fifo.nodes_expanded);
    CHECK(lifo.nodes_generated >= lifo.nodes_expanded);
    CHECK(least_cost.nodes_generated >= least_cost.nodes_expanded);

    const std::vector<project47::KnapsackItem> dp_items{
        {"tile-a", 10, 60}, {"tile-b", 20, 100},
        {"tile-c", 30, 120}, {"tile-d", 15, 75}
    };
    const auto dynamic = project47::zero_one_knapsack(dp_items, 50);
    CHECK(fifo.total_value == dynamic.total_value);

    const auto zero_weight = project47::branch_and_bound_knapsack(
        {{"metadata-index", 0, 9}, {"tile", 2, 4}}, 0,
        project47::BranchAndBoundStrategy::LeastCost);
    CHECK(zero_weight.total_value == 9);
    CHECK(zero_weight.total_weight == 0);
    CHECK((zero_weight.selected_indices == std::vector<std::size_t>{0}));
    const auto empty = project47::branch_and_bound_knapsack(
        {}, 10, project47::BranchAndBoundStrategy::FIFO);
    CHECK(empty.total_value == 0);
    CHECK(empty.selected_indices.empty());
    check_throws<std::invalid_argument>([] {
        (void)project47::branch_and_bound_knapsack(
            {{"same", 1, 1}, {"same", 2, 2}}, 2,
            project47::BranchAndBoundStrategy::FIFO);
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::branch_and_bound_knapsack(
            {{"", 1, 1}}, 2, project47::BranchAndBoundStrategy::FIFO);
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::branch_and_bound_knapsack(
            {{"tile", 1, 1}}, 1, static_cast<project47::BranchAndBoundStrategy>(99));
    });
}

void test_branch_and_bound_randomized_against_dynamic_programming() {
    std::mt19937 generator(47);
    std::uniform_int_distribution<int> count_distribution(0, 8);
    std::uniform_int_distribution<int> weight_distribution(0, 15);
    std::uniform_int_distribution<int> value_distribution(0, 40);
    std::uniform_int_distribution<int> capacity_distribution(0, 30);
    for (int sample = 0; sample < 30; ++sample) {
        const int count = count_distribution(generator);
        std::vector<project47::BnBItem> bnb_items;
        std::vector<project47::KnapsackItem> dp_items;
        for (int i = 0; i < count; ++i) {
            const auto weight = static_cast<std::size_t>(weight_distribution(generator));
            const auto value = static_cast<std::uint64_t>(value_distribution(generator));
            const std::string id = "item-" + std::to_string(i);
            bnb_items.push_back({id, weight, value});
            dp_items.push_back({id, weight, value});
        }
        const auto capacity = static_cast<std::size_t>(capacity_distribution(generator));
        const auto expected = project47::zero_one_knapsack(dp_items, capacity);
        for (const auto strategy : {project47::BranchAndBoundStrategy::FIFO,
                                    project47::BranchAndBoundStrategy::LIFO,
                                    project47::BranchAndBoundStrategy::LeastCost}) {
            const auto actual = project47::branch_and_bound_knapsack(bnb_items, capacity, strategy);
            CHECK(actual.total_value == expected.total_value);
            CHECK(actual.total_weight <= capacity);
            std::uint64_t selected_value = 0;
            std::size_t selected_weight = 0;
            std::vector<bool> seen(bnb_items.size(), false);
            for (const auto index : actual.selected_indices) {
                CHECK(index < bnb_items.size());
                CHECK(!seen[index]);
                seen[index] = true;
                selected_value += bnb_items[index].value;
                selected_weight += bnb_items[index].weight;
            }
            CHECK(selected_value == actual.total_value);
            CHECK(selected_weight == actual.total_weight);
        }
    }
}


void test_ford_fulkerson_max_flow() {
    project47::FlowNetwork network;
    for (const auto& vertex : {"s", "a", "b", "t"}) network.add_vertex(vertex);
    network.add_edge("s", "a", 10);
    network.add_edge("s", "b", 5);
    network.add_edge("a", "b", 15);
    network.add_edge("a", "t", 10);
    network.add_edge("b", "t", 10);
    const auto result = network.ford_fulkerson_max_flow("s", "t");
    CHECK(result.maximum_flow == 15);
    CHECK(result.augmentations >= 2);
    CHECK(std::find(result.source_side_of_min_cut.begin(),
                    result.source_side_of_min_cut.end(), "s") != result.source_side_of_min_cut.end());
    CHECK(std::find(result.source_side_of_min_cut.begin(),
                    result.source_side_of_min_cut.end(), "t") == result.source_side_of_min_cut.end());

    project47::FlowNetwork empty_path;
    empty_path.add_vertex("source");
    empty_path.add_vertex("sink");
    const auto no_flow = empty_path.ford_fulkerson_max_flow("source", "sink");
    CHECK(no_flow.maximum_flow == 0);
    CHECK(no_flow.augmentations == 0);
    check_throws<std::invalid_argument>([&] {
        (void)network.ford_fulkerson_max_flow("missing", "t");
    });
    check_throws<std::invalid_argument>([&] {
        (void)network.ford_fulkerson_max_flow("s", "s");
    });
    check_throws<std::invalid_argument>([&] { network.add_edge("s", "missing", 1); });
    check_throws<std::invalid_argument>([&] { network.add_edge("s", "s", 1); });
    check_throws<std::invalid_argument>([] {
        project47::FlowNetwork invalid;
        invalid.add_vertex("");
    });

    project47::FlowNetwork overflow_network;
    overflow_network.add_vertex("source");
    overflow_network.add_vertex("sink");
    overflow_network.add_edge("source", "sink", std::numeric_limits<std::uint64_t>::max());
    overflow_network.add_edge("source", "sink", std::numeric_limits<std::uint64_t>::max());
    check_throws<std::overflow_error>([&] {
        (void)overflow_network.ford_fulkerson_max_flow("source", "sink");
    });
}

void test_maximum_bipartite_matching() {
    project47::BipartiteGraph graph;
    for (const auto& id : {"tile-a", "tile-b", "tile-c"}) graph.add_left_vertex(id);
    for (const auto& id : {"compute-1", "compute-2", "compute-3"}) graph.add_right_vertex(id);
    CHECK(graph.add_edge("tile-a", "compute-1"));
    CHECK(graph.add_edge("tile-a", "compute-2"));
    CHECK(graph.add_edge("tile-b", "compute-1"));
    CHECK(graph.add_edge("tile-c", "compute-2"));
    CHECK(graph.add_edge("tile-c", "compute-3"));
    CHECK(!graph.add_edge("tile-c", "compute-3"));
    const auto matching = graph.maximum_matching();
    CHECK(graph.left_count() == 3);
    CHECK(graph.right_count() == 3);
    CHECK(matching.cardinality() == 3);
    std::set<std::string> matched_left;
    std::set<std::string> matched_right;
    for (const auto& pair : matching.pairs) {
        CHECK(matched_left.insert(pair.left).second);
        CHECK(matched_right.insert(pair.right).second);
    }

    project47::BipartiteGraph constrained;
    constrained.add_left_vertex("tile-1");
    constrained.add_left_vertex("tile-2");
    constrained.add_right_vertex("worker-1");
    constrained.add_edge("tile-1", "worker-1");
    constrained.add_edge("tile-2", "worker-1");
    CHECK(constrained.maximum_matching().cardinality() == 1);
    CHECK(project47::BipartiteGraph{}.maximum_matching().cardinality() == 0);
    check_throws<std::invalid_argument>([] {
        project47::BipartiteGraph invalid;
        invalid.add_edge("unknown-left", "unknown-right");
    });
    check_throws<std::invalid_argument>([] {
        project47::BipartiteGraph invalid;
        invalid.add_left_vertex("");
    });
}

void test_chinese_remainder_theorem() {
    const auto classic = project47::chinese_remainder_theorem({{2, 3}, {3, 5}, {2, 7}});
    CHECK(classic.residue == 23);
    CHECK(classic.modulus == 105);
    CHECK(classic.residue % 3 == 2);
    CHECK(classic.residue % 5 == 3);
    CHECK(classic.residue % 7 == 2);

    const auto negative = project47::chinese_remainder_theorem({{-1, 4}, {2, 5}});
    CHECK(negative.residue == 7);
    CHECK(negative.modulus == 20);
    const auto single = project47::chinese_remainder_theorem({{42, 11}});
    CHECK(single.residue == 9);
    CHECK(single.modulus == 11);

    check_throws<std::invalid_argument>([] {
        (void)project47::chinese_remainder_theorem({});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::chinese_remainder_theorem({{1, 1}});
    });
    check_throws<std::invalid_argument>([] {
        (void)project47::chinese_remainder_theorem({{1, 6}, {2, 9}});
    });
    check_throws<std::overflow_error>([] {
        (void)project47::chinese_remainder_theorem({{0, 4'000'000'007LL},
                                                     {0, 4'000'000'009LL}});
    });
}

void test_string_matching_algorithms() {
    const std::string text = "tile_id_12_tile_id_12";
    const std::string pattern = "tile_id";
    const std::vector<std::size_t> expected{0, 11};
    CHECK(project47::naive_string_search(text, pattern) == expected);
    CHECK(project47::rabin_karp_search(text, pattern) == expected);
    CHECK(project47::finite_automaton_search(text, pattern) == expected);
    CHECK(project47::kmp_search(text, pattern) == expected);

    const std::string overlap_text = "aaaaa";
    const std::string overlap_pattern = "aaa";
    const std::vector<std::size_t> overlap_expected{0, 1, 2};
    CHECK(project47::naive_string_search(overlap_text, overlap_pattern) == overlap_expected);
    CHECK(project47::rabin_karp_search(overlap_text, overlap_pattern) == overlap_expected);
    CHECK(project47::finite_automaton_search(overlap_text, overlap_pattern) == overlap_expected);
    CHECK(project47::kmp_search(overlap_text, overlap_pattern) == overlap_expected);

    const std::string binary_text("a\0b\0a", 5);
    const std::string binary_pattern("\0b", 2);
    const std::vector<std::size_t> binary_expected{1};
    CHECK(project47::naive_string_search(binary_text, binary_pattern) == binary_expected);
    CHECK(project47::rabin_karp_search(binary_text, binary_pattern) == binary_expected);
    CHECK(project47::finite_automaton_search(binary_text, binary_pattern) == binary_expected);
    CHECK(project47::kmp_search(binary_text, binary_pattern) == binary_expected);

    const std::vector<std::size_t> empty_expected{0, 1, 2, 3};
    CHECK(project47::naive_string_search("abc", "") == empty_expected);
    CHECK(project47::rabin_karp_search("abc", "") == empty_expected);
    CHECK(project47::finite_automaton_search("abc", "") == empty_expected);
    CHECK(project47::kmp_search("abc", "") == empty_expected);
    CHECK(project47::naive_string_search("abc", "longer").empty());
    CHECK(project47::rabin_karp_search("abc", "longer").empty());
    CHECK(project47::finite_automaton_search("abc", "longer").empty());
    CHECK(project47::kmp_search("abc", "longer").empty());
    CHECK(project47::naive_string_search("abc", "z").empty());
    CHECK(project47::rabin_karp_search("abc", "z").empty());
    CHECK(project47::finite_automaton_search("abc", "z").empty());
    CHECK(project47::kmp_search("abc", "z").empty());
}

void test_scheduler_route_cache_runs_dijkstra_once_per_online_worker_plus_source() {
    const auto workload = project47::make_demo_workload();
    const auto cache = project47::detail::build_route_cache(workload);
    const std::size_t online_workers = static_cast<std::size_t>(std::count_if(
        workload.compute_nodes.begin(), workload.compute_nodes.end(),
        [](const project47::ComputeNode& node) {
            return node.status == project47::ComputeNodeStatus::Online;
        }));
    CHECK(cache.dijkstra_runs == online_workers + 1);
    CHECK(cache.by_compute_node.size() == online_workers);
    for (const auto& [compute_id, route] : cache.by_compute_node) {
        CHECK(!compute_id.empty());
        CHECK(!route.input_route.empty());
        CHECK(!route.output_route.empty());
        CHECK(route.input_route.front() == workload.input_storage_id);
        CHECK(route.input_route.back() == compute_id);
        CHECK(route.output_route.front() == compute_id);
        CHECK(route.output_route.back() == workload.output_storage_id);
    }
}

void test_integrated_scheduler_normal_scenario() {
    const auto result = project47::run_scheduler(project47::make_demo_workload(),
                                                  project47::Scenario::Normal);
    CHECK(result.scenario == "normal");
    CHECK(result.total_jobs == 5);
    CHECK(result.completed_count() == 5);
    CHECK(result.deferred_count() == 0);
    CHECK(result.waves == 2);
    CHECK(result.matching_assignments == 5);
    CHECK(result.total_estimated_processing_seconds == 53.0);
    CHECK(result.estimated_makespan_seconds == 27.0);
    CHECK(result.total_route_cost > 0.0);

    std::map<std::uint64_t, std::size_t> wave_by_job;
    std::map<std::size_t, std::set<std::string>> workers_by_wave;
    for (const auto& execution : result.completed_jobs) {
        wave_by_job.emplace(execution.job.id, execution.wave);
        CHECK(!execution.input_route.empty());
        CHECK(!execution.output_route.empty());
        CHECK(execution.input_route.front() == execution.source_storage_id);
        CHECK(execution.input_route.back() == execution.compute_node_id);
        CHECK(execution.output_route.front() == execution.compute_node_id);
        CHECK(execution.output_route.back() == execution.destination_storage_id);
        CHECK(workers_by_wave[execution.wave].insert(execution.compute_node_id).second);
    }
    CHECK(wave_by_job.at(101) < wave_by_job.at(104));
    CHECK(wave_by_job.at(102) < wave_by_job.at(104));
    CHECK(wave_by_job.at(103) < wave_by_job.at(105));
    CHECK(result.final_storage_used_bytes.at("storage-archive") == 45'000'000);
}

void test_integrated_scheduler_capacity_shock() {
    const auto normal_workload = project47::make_demo_workload(project47::Scenario::Normal);
    const auto shock_workload = project47::make_demo_workload(project47::Scenario::CapacityShock);
    CHECK(normal_workload.jobs.size() == shock_workload.jobs.size());
    CHECK(normal_workload.dependencies == shock_workload.dependencies);
    for (std::size_t i = 0; i < normal_workload.jobs.size(); ++i) {
        CHECK(normal_workload.jobs[i].id == shock_workload.jobs[i].id);
        CHECK(normal_workload.jobs[i].input_bytes == shock_workload.jobs[i].input_bytes);
    }
    const auto result = project47::run_scheduler(shock_workload, project47::Scenario::CapacityShock);
    CHECK(result.scenario == "capacity-shock");
    CHECK(result.total_jobs == 5);
    CHECK(result.completed_count() == 2);
    CHECK(result.deferred_count() == 3);
    CHECK(result.waves == 2);
    CHECK(result.final_storage_used_bytes.at("storage-archive") ==
          2ULL * 1024 * 1024 * 1024 - 4'000'000);
    CHECK(result.deferred_jobs.front().id == 102);
}

void test_integrated_scheduler_stress_workload() {
    const auto workload = project47::make_stress_workload(40);
    const auto result = project47::run_scheduler(workload, project47::Scenario::Stress);
    CHECK(result.total_jobs == 40);
    CHECK(result.completed_count() == 40);
    CHECK(result.deferred_count() == 0);
    CHECK(result.waves == 5);
    CHECK(result.matching_assignments == 40);
    CHECK(result.final_storage_used_bytes.at("stress-archive") > 0);
}

void test_workload_csv_loader_and_validation() {
    std::istringstream csv(R"csv(id,tile_id,priority,estimated_seconds,input_bytes,prerequisites
501,"tile,501",5,12.5,1024,
502,tile_502,3,8.0,2048,501
503,tile_503,2,4.0,4096,501;502
)csv");
    const auto loaded = project47::load_jobs_csv(csv);
    CHECK(loaded.jobs.size() == 3);
    CHECK(loaded.jobs[0].tile_id == "tile,501");
    CHECK(loaded.dependencies.size() == 3);
    CHECK((loaded.dependencies[0] == std::pair<std::uint64_t, std::uint64_t>{501, 502}));
    CHECK((loaded.dependencies[1] == std::pair<std::uint64_t, std::uint64_t>{501, 503}));
    CHECK((loaded.dependencies[2] == std::pair<std::uint64_t, std::uint64_t>{502, 503}));

    std::istringstream malformed("id,tile_id,priority,estimated_seconds,input_bytes\n"
                                 "x,tile,1,2.0,100\n");
    check_throws<std::invalid_argument>([&] { (void)project47::load_jobs_csv(malformed); });
    std::istringstream unterminated(R"csv(1,"bad,1,2.0,100
)csv");
    check_throws<std::invalid_argument>([&] { (void)project47::load_jobs_csv(unterminated); });
}

void test_scheduler_rejects_dependency_cycles_and_missing_tiles() {
    auto workload = project47::make_demo_workload();
    workload.dependencies.push_back({104, 101});
    check_throws<std::logic_error>([&] { (void)project47::run_scheduler(workload); });

    workload = project47::make_demo_workload();
    workload.jobs[0].tile_id = "missing-tile";
    check_throws<std::invalid_argument>([&] { (void)project47::run_scheduler(workload); });
}

void test_scheduler_report_export_formats() {
    const auto result = project47::run_scheduler(project47::make_demo_workload());
    std::ostringstream json;
    std::ostringstream csv;
    project47::write_json_report(result, json);
    project47::write_jobs_csv(result, csv);
    CHECK(json.str().find("\"scenario\": \"normal\"") != std::string::npos);
    CHECK(json.str().find("\"completed_jobs\": 5") != std::string::npos);
    CHECK(json.str().find("\"estimated_makespan_seconds\": 27") != std::string::npos);
    CHECK(json.str().find("\"makespan_model\": \"wave_barrier_estimate\"") != std::string::npos);
    CHECK(csv.str().find("scenario,job_id,tile_id,status") == 0);
    CHECK(csv.str().find("completed") != std::string::npos);

    const auto output_dir = std::filesystem::temp_directory_path() / "project47_phase8_test_reports";
    std::filesystem::remove_all(output_dir);
    project47::export_scheduler_reports(result, output_dir);
    CHECK(std::filesystem::exists(output_dir / "normal_jobs.csv"));
    CHECK(std::filesystem::exists(output_dir / "normal_report.json"));
    std::filesystem::remove_all(output_dir);
}

void test_maximum_subarray_divide_and_conquer() {
    const auto result = project47::maximum_subarray_divide_conquer(
        {-2, 1, -3, 4, -1, 2, 1, -5, 4});
    CHECK(result.sum == 6);
    CHECK(result.begin == 3);
    CHECK(result.end == 7);
    const auto all_negative = project47::maximum_subarray_divide_conquer({-8, -3, -5});
    CHECK(all_negative.sum == -3);
    CHECK(all_negative.begin == 1);
    CHECK(all_negative.end == 2);
    check_throws<std::invalid_argument>([] {
        (void)project47::maximum_subarray_divide_conquer({});
    });
}

void test_fast_power() {
    CHECK(std::abs(project47::fast_power(2.0, 10) - 1024.0) < 1e-9);
    CHECK(std::abs(project47::fast_power(2.0, -3) - 0.125) < 1e-9);
    CHECK(std::abs(project47::fast_power(-3.0, 3) + 27.0) < 1e-9);
    CHECK(std::abs(project47::fast_power(7.0, 0) - 1.0) < 1e-9);
    check_throws<std::domain_error>([] { (void)project47::fast_power(0.0, -1); });
    check_throws<std::invalid_argument>([] {
        (void)project47::fast_power(std::numeric_limits<double>::infinity(), 2);
    });
}

void test_celebrity_discovery() {
    std::vector<std::vector<bool>> knows(4, std::vector<bool>(4, false));
    for (std::size_t person = 0; person < 4; ++person) {
        if (person != 2) knows[person][2] = true;
    }
    CHECK(project47::find_celebrity(knows) == 2);
    CHECK(project47::find_celebrity_divide_conquer(knows) == 2);
    CHECK(!project47::find_celebrity({{false, false}, {false, false}}).has_value());
    CHECK(!project47::find_celebrity_divide_conquer({{false, false}, {false, false}}).has_value());
    CHECK(!project47::find_celebrity({}).has_value());
    CHECK(!project47::find_celebrity_divide_conquer({}).has_value());
    check_throws<std::invalid_argument>([] {
        (void)project47::find_celebrity_divide_conquer({{false, true}, {false}});
    });

    std::mt19937 generator(9447);
    std::bernoulli_distribution relation(0.5);
    for (std::size_t size = 1; size <= 12; ++size) {
        for (int sample = 0; sample < 40; ++sample) {
            std::vector<std::vector<bool>> matrix(size, std::vector<bool>(size, false));
            for (std::size_t row = 0; row < size; ++row) {
                for (std::size_t column = 0; column < size; ++column) {
                    if (row != column) matrix[row][column] = relation(generator);
                }
            }
            CHECK(project47::find_celebrity_divide_conquer(matrix) ==
                  project47::find_celebrity(matrix));
        }
    }
}

void test_classical_and_strassen_matrix_multiplication() {
    project47::IntMatrix a(64, std::vector<std::int64_t>(64, 0));
    project47::IntMatrix b(64, std::vector<std::int64_t>(64, 0));
    for (std::size_t i = 0; i < 64; ++i) {
        for (std::size_t j = 0; j < 64; ++j) {
            a[i][j] = static_cast<std::int64_t>((i + 2 * j) % 7) - 3;
            b[i][j] = static_cast<std::int64_t>((3 * i + j) % 9) - 4;
        }
    }
    const auto classical = project47::classical_matrix_multiply(a, b);
    const auto strassen = project47::strassen_matrix_multiply(a, b);
    CHECK(classical == strassen);

    const project47::IntMatrix odd_a{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const project47::IntMatrix odd_b{{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    CHECK(project47::classical_matrix_multiply(odd_a, odd_b) ==
          project47::strassen_matrix_multiply(odd_a, odd_b));
    check_throws<std::invalid_argument>([] {
        (void)project47::strassen_matrix_multiply({{1, 2}}, {{1, 2}});
    });
}

void test_bounded_tsp_held_karp() {
    const double inf = std::numeric_limits<double>::infinity();
    const std::vector<std::vector<double>> costs{
        {inf, 10, 15, 20}, {10, inf, 35, 25},
        {15, 35, inf, 30}, {20, 25, 30, inf}
    };
    const auto result = project47::travelling_salesperson_held_karp(costs);
    CHECK(result.feasible());
    CHECK(std::abs(result.minimum_cost - 80.0) < 1e-9);
    CHECK(result.tour.size() == 5);
    CHECK(result.tour.front() == 0);
    CHECK(result.tour.back() == 0);
    auto unique = result.tour;
    unique.pop_back();
    std::sort(unique.begin(), unique.end());
    CHECK((unique == std::vector<std::size_t>{0, 1, 2, 3}));

    const auto singleton = project47::travelling_salesperson_held_karp({{0.0}});
    CHECK(singleton.feasible());
    CHECK(singleton.minimum_cost == 0.0);
    CHECK((singleton.tour == std::vector<std::size_t>{0, 0}));

    const auto impossible = project47::travelling_salesperson_held_karp(
        {{inf, 1, inf}, {inf, inf, 1}, {inf, inf, inf}});
    CHECK(!impossible.feasible());
    check_throws<std::invalid_argument>([] {
        (void)project47::travelling_salesperson_held_karp({});
    });
    check_throws<std::out_of_range>([] {
        (void)project47::travelling_salesperson_held_karp({{0.0, 1.0}, {1.0, 0.0}}, 2);
    });
}

void test_string_matching_randomized_against_naive() {
    std::mt19937 generator(4707);
    std::uniform_int_distribution<int> text_length(0, 45);
    std::uniform_int_distribution<int> pattern_length(0, 12);
    std::uniform_int_distribution<int> letter(0, 4);
    for (int sample = 0; sample < 60; ++sample) {
        std::string text;
        std::string pattern;
        const int text_size = text_length(generator);
        const int pattern_size = pattern_length(generator);
        for (int i = 0; i < text_size; ++i) text.push_back(static_cast<char>('a' + letter(generator)));
        for (int i = 0; i < pattern_size; ++i) pattern.push_back(static_cast<char>('a' + letter(generator)));
        const auto expected = project47::naive_string_search(text, pattern);
        CHECK(project47::rabin_karp_search(text, pattern) == expected);
        CHECK(project47::finite_automaton_search(text, pattern) == expected);
        CHECK(project47::kmp_search(text, pattern) == expected);
    }
}

} // namespace

int main() {
    try {
        test_merge_sort_integers();
        test_merge_sort_empty_single_and_stability();
        test_job_scheduling_order();
        test_invalid_job_is_rejected();
        test_quick_sort_basic_and_duplicates();
        test_quick_sort_edge_cases();
        test_binary_search_variants();
        test_tile_validation();
        test_compute_node_validation();
        test_storage_node_validation_and_capacity();
        test_dependency_graph_order_and_duplicates();
        test_dependency_graph_rejects_invalid_edges_and_cycles();
        test_activity_selection_greedy();
        test_interval_partitioning_minimum_resources();
        test_huffman_encoding_and_validation();
        test_weighted_graph_validation_and_dijkstra();
        test_prim_and_kruskal_minimum_spanning_tree();
        test_zero_one_knapsack();
        test_matrix_chain_multiplication();
        test_longest_common_subsequence();
        test_optimal_binary_search_tree();
        test_bellman_ford_negative_edges_and_cycles();
        test_floyd_warshall_all_pairs();
        test_n_queens_backtracking();
        test_subset_sum_backtracking();
        test_branch_and_bound_strategies_match_dynamic_programming();
        test_branch_and_bound_randomized_against_dynamic_programming();
        test_ford_fulkerson_max_flow();
        test_maximum_bipartite_matching();
        test_chinese_remainder_theorem();
        test_string_matching_algorithms();
        test_scheduler_route_cache_runs_dijkstra_once_per_online_worker_plus_source();
        test_integrated_scheduler_normal_scenario();
        test_integrated_scheduler_capacity_shock();
        test_integrated_scheduler_stress_workload();
        test_workload_csv_loader_and_validation();
        test_scheduler_rejects_dependency_cycles_and_missing_tiles();
        test_scheduler_report_export_formats();
        test_string_matching_randomized_against_naive();
        test_maximum_subarray_divide_and_conquer();
        test_fast_power();
        test_celebrity_discovery();
        test_classical_and_strassen_matrix_multiplication();
        test_bounded_tsp_held_karp();
    } catch (const std::exception& error) {
        std::cerr << "TEST FAILURE: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All Project 47 checks passed (" << checks_run << " assertions).\n";
    return 0;
}
