#include "project47/algorithms/merge_sort.hpp"
#include "project47/algorithms/branch_and_bound.hpp"
#include "project47/algorithms/knapsack.hpp"
#include "project47/algorithms/quick_sort.hpp"
#include "project47/algorithms/string_matching.hpp"
#include "project47/scheduler/scheduler.hpp"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;

std::int64_t median_microseconds(std::vector<std::int64_t> samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

template <typename Function>
std::int64_t measure_median(Function&& function, int repeats = 3) {
    std::vector<std::int64_t> samples;
    samples.reserve(static_cast<std::size_t>(repeats));
    for (int repeat = 0; repeat < repeats; ++repeat) {
        const auto start = Clock::now();
        function();
        const auto stop = Clock::now();
        samples.push_back(std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count());
    }
    return median_microseconds(std::move(samples));
}

std::vector<std::int64_t> make_data(std::size_t size, const std::string& shape) {
    std::vector<std::int64_t> values(size);
    std::mt19937_64 generator(470047);
    if (shape == "random") {
        std::uniform_int_distribution<std::int64_t> distribution(-1'000'000, 1'000'000);
        for (auto& value : values) value = distribution(generator);
    } else if (shape == "sorted") {
        std::iota(values.begin(), values.end(), 0);
    } else if (shape == "reverse") {
        std::iota(values.begin(), values.end(), 0);
        std::reverse(values.begin(), values.end());
    } else if (shape == "duplicates") {
        std::uniform_int_distribution<std::int64_t> distribution(0, 7);
        for (auto& value : values) value = distribution(generator);
    } else if (shape == "organ-pipe") {
        for (std::size_t i = 0; i < size; ++i) {
            values[i] = static_cast<std::int64_t>(std::min(i, size - i));
        }
    } else {
        throw std::invalid_argument("Unknown benchmark data shape");
    }
    return values;
}

std::int64_t checksum(const std::vector<std::int64_t>& values) {
    std::int64_t result = 0;
    for (const auto value : values) result = (result * 131) ^ value;
    return result;
}

void write_sort_benchmarks(std::ofstream& csv, const std::vector<std::size_t>& sizes) {
    const std::vector<std::string> shapes{"random", "sorted", "reverse", "duplicates", "organ-pipe"};
    for (const std::size_t size : sizes) {
        for (const auto& shape : shapes) {
            const auto input = make_data(size, shape);
            std::int64_t merge_checksum = 0;
            const auto merge_time = measure_median([&] {
                auto values = input;
                project47::merge_sort(values);
                if (!std::is_sorted(values.begin(), values.end())) {
                    throw std::runtime_error("Merge Sort correctness check failed");
                }
                merge_checksum = checksum(values);
            });
            std::int64_t quick_checksum = 0;
            const auto quick_time = measure_median([&] {
                auto values = input;
                project47::quick_sort(values);
                if (!std::is_sorted(values.begin(), values.end())) {
                    throw std::runtime_error("Quick Sort correctness check failed");
                }
                quick_checksum = checksum(values);
            });
            const bool same_result = merge_checksum == quick_checksum;
            if (!same_result) throw std::runtime_error("Sort implementations disagree");
            const auto merge_aux = static_cast<std::uint64_t>(size) * sizeof(std::int64_t);
            const auto quick_aux = static_cast<std::uint64_t>(
                2 * (1 + static_cast<std::size_t>(std::bit_width(size)))) * sizeof(std::size_t);
            csv << "sorting," << shape << ',' << size << ",merge_sort," << merge_time
                << ",pass," << merge_aux << ",median of 3 runs; auxiliary buffer estimate\n";
            csv << "sorting," << shape << ',' << size << ",quick_sort," << quick_time
                << ",pass," << quick_aux << ",median of 3 runs; recursion-stack estimate; not measured RSS\n";
            std::cout << "Sort " << shape << " n=" << size << ": merge " << merge_time
                      << " us, quick " << quick_time << " us\n";
        }
    }
}

void write_string_benchmarks(std::ofstream& csv, const std::vector<std::size_t>& sizes) {
    constexpr std::size_t pattern_size = 48;
    const std::string pattern = std::string(pattern_size - 1, 'a') + 'b';
    for (const std::size_t size : sizes) {
        const std::string text(size, 'a');
        std::vector<std::size_t> expected;
        const auto run = [&](const std::string& name, auto search, std::uint64_t auxiliary_bytes) {
            std::vector<std::size_t> result;
            const auto elapsed = measure_median([&] { result = search(text, pattern); });
            if (result != expected) throw std::runtime_error(name + " string search disagrees with Naive");
            csv << "string_search,repetitive," << size << ',' << name << ',' << elapsed
                << ",pass," << auxiliary_bytes << ",pattern length " << pattern_size
                << "; median of 3 runs; excludes returned match-vector storage\n";
            std::cout << "String " << name << " n=" << size << ": " << elapsed << " us\n";
        };
        expected = project47::naive_string_search(text, pattern);
        run("naive", project47::naive_string_search, 0);
        run("rabin_karp", project47::rabin_karp_search, 3 * sizeof(std::uint64_t));
        run("kmp", project47::kmp_search, pattern_size * sizeof(std::size_t));
        run("finite_automaton", project47::finite_automaton_search,
            pattern_size * 256ULL * sizeof(std::size_t));
    }
}


void write_exact_search_benchmarks(std::ofstream& csv) {
    std::vector<project47::BnBItem> bnb_items;
    std::vector<project47::KnapsackItem> dp_items;
    for (std::size_t i = 0; i < 22; ++i) {
        const std::size_t weight = 4 + (i * 7) % 23;
        const std::uint64_t value = 13 + static_cast<std::uint64_t>((i * 29) % 91);
        const std::string id = "tile-bench-" + std::to_string(i);
        bnb_items.push_back({id, weight, value});
        dp_items.push_back({id, weight, value});
    }
    constexpr std::size_t capacity = 100;
    const auto oracle = project47::zero_one_knapsack(dp_items, capacity);
    csv << "exact_search,capacity-" << capacity << ',' << bnb_items.size()
        << ",dynamic_programming_oracle,"
        << measure_median([&] { (void)project47::zero_one_knapsack(dp_items, capacity); }, 5)
        << ",pass,,median of 5 runs; exact optimum value=" << oracle.total_value << '\n';
    for (const auto strategy : {project47::BranchAndBoundStrategy::FIFO,
                                project47::BranchAndBoundStrategy::LIFO,
                                project47::BranchAndBoundStrategy::LeastCost}) {
        project47::BranchAndBoundResult final_result;
        const auto elapsed = measure_median([&] {
            final_result = project47::branch_and_bound_knapsack(bnb_items, capacity, strategy);
        }, 5);
        if (final_result.total_value != oracle.total_value) {
            throw std::runtime_error("Branch-and-bound result differs from DP optimum");
        }
        csv << "exact_search,capacity-" << capacity << ',' << bnb_items.size()
            << ",branch_and_bound_" << project47::to_string(strategy) << ',' << elapsed
            << ",pass,,median of 5 runs; value=" << final_result.total_value
            << "; weight=" << final_result.total_weight
            << "; expanded_nodes=" << final_result.nodes_expanded
            << "; generated_nodes=" << final_result.nodes_generated << '\n';
        std::cout << "Exact search " << project47::to_string(strategy)
                  << ": " << elapsed << " us; value " << final_result.total_value
                  << "; expanded " << final_result.nodes_expanded << " nodes\n";
    }
}

void write_scheduler_benchmarks(std::ofstream& csv) {
    struct Case { project47::Scenario scenario; std::size_t jobs; };
    const std::vector<Case> cases{
        {project47::Scenario::Normal, 5},
        {project47::Scenario::CapacityShock, 5},
        {project47::Scenario::Stress, 128}
    };
    for (const auto& item : cases) {
        project47::SchedulerResult last_result;
        const auto elapsed = measure_median([&] {
            auto workload = item.scenario == project47::Scenario::Stress
                ? project47::make_stress_workload(item.jobs)
                : project47::make_demo_workload(item.scenario);
            last_result = project47::run_scheduler(std::move(workload), item.scenario);
        }, 5);
        if (last_result.total_jobs != item.jobs ||
            last_result.completed_count() + last_result.deferred_count() != item.jobs) {
            throw std::runtime_error("Scheduler benchmark accounting check failed");
        }
        csv << "scheduler," << project47::to_string(item.scenario) << ',' << item.jobs
            << ",integrated_scheduler," << elapsed << ",pass,,median of 5 runs; host-dependent wall time"
            << "; completed=" << last_result.completed_count()
            << "; deferred=" << last_result.deferred_count()
            << "; waves=" << last_result.waves << '\n';
        std::cout << "Scheduler " << project47::to_string(item.scenario) << " jobs=" << item.jobs
                  << ": " << elapsed << " us; completed " << last_result.completed_count()
                  << ", deferred " << last_result.deferred_count() << "\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        const std::filesystem::path output = argc > 1 ? argv[1] : "outputs/benchmark_results.csv";
        if (argc > 2) {
            std::cerr << "Usage: project47_benchmarks [OUTPUT.csv]\n";
            return 2;
        }
        if (output.has_parent_path()) std::filesystem::create_directories(output.parent_path());
        std::ofstream csv(output);
        if (!csv) throw std::runtime_error("Cannot open benchmark output: " + output.string());
        csv << "category,dataset,input_size,algorithm,median_microseconds,correctness,"
               "estimated_auxiliary_bytes,notes\n";
        std::cout << "Project 47 reproducible benchmark suite\n"
                  << "Compiler: " << __VERSION__ << " | C++ " << __cplusplus << "\n"
                  << "Timings are local observations, not portable performance guarantees.\n";
        write_sort_benchmarks(csv, {1'000, 10'000, 50'000});
        write_string_benchmarks(csv, {1'000, 10'000, 50'000});
        write_exact_search_benchmarks(csv);
        write_scheduler_benchmarks(csv);
        csv.flush();
        if (!csv) throw std::runtime_error("Failed while writing benchmark output");
        std::cout << "Benchmark CSV: " << output << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
