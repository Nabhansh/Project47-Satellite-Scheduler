#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace project47 {

struct TspResult {
    double minimum_cost{std::numeric_limits<double>::infinity()};
    std::vector<std::size_t> tour; // Includes the start city at both ends.
    [[nodiscard]] bool feasible() const noexcept { return !tour.empty(); }
};

// Held-Karp dynamic programming for a bounded TSP instance. Infinity denotes
// a missing directed edge. Supports at most 18 cities due to O(n 2^n) memory.
// Time O(n^2 2^n); space O(n 2^n).
inline TspResult travelling_salesperson_held_karp(
    const std::vector<std::vector<double>>& costs, std::size_t start = 0) {
    const std::size_t n = costs.size();
    if (n == 0 || n > 18) throw std::invalid_argument("TSP requires 1..18 cities");
    if (start >= n) throw std::out_of_range("TSP start city is out of range");
    for (const auto& row : costs) {
        if (row.size() != n) throw std::invalid_argument("TSP cost matrix must be square");
        for (const double cost : row) {
            if (std::isnan(cost) || cost == -std::numeric_limits<double>::infinity()) {
                throw std::invalid_argument("TSP edge costs must be finite or positive infinity");
            }
        }
    }
    if (n == 1) return {0.0, {start, start}};

    const std::size_t state_count = std::size_t{1} << n;
    const double infinity = std::numeric_limits<double>::infinity();
    std::vector<double> dp(state_count * n, infinity);
    std::vector<std::size_t> parent(state_count * n, n);
    const std::size_t start_mask = std::size_t{1} << start;
    dp[start_mask * n + start] = 0.0;

    for (std::size_t mask = 0; mask < state_count; ++mask) {
        if ((mask & start_mask) == 0) continue;
        for (std::size_t from = 0; from < n; ++from) {
            if ((mask & (std::size_t{1} << from)) == 0) continue;
            const double current = dp[mask * n + from];
            if (!std::isfinite(current)) continue;
            for (std::size_t to = 0; to < n; ++to) {
                const std::size_t bit = std::size_t{1} << to;
                if ((mask & bit) != 0 || !std::isfinite(costs[from][to])) continue;
                const std::size_t next_mask = mask | bit;
                const std::size_t index = next_mask * n + to;
                const double candidate = current + costs[from][to];
                if (candidate < dp[index]) {
                    dp[index] = candidate;
                    parent[index] = from;
                }
            }
        }
    }

    const std::size_t full_mask = state_count - 1;
    double best = infinity;
    std::size_t last = n;
    for (std::size_t city = 0; city < n; ++city) {
        if (city == start || !std::isfinite(dp[full_mask * n + city]) ||
            !std::isfinite(costs[city][start])) continue;
        const double candidate = dp[full_mask * n + city] + costs[city][start];
        if (candidate < best) {
            best = candidate;
            last = city;
        }
    }
    if (last == n) return {};

    std::vector<std::size_t> reverse_path;
    std::size_t mask = full_mask;
    std::size_t city = last;
    while (city != start) {
        reverse_path.push_back(city);
        const std::size_t previous = parent[mask * n + city];
        if (previous == n) throw std::logic_error("TSP parent reconstruction failed");
        mask ^= (std::size_t{1} << city);
        city = previous;
    }
    TspResult result;
    result.minimum_cost = best;
    result.tour.push_back(start);
    for (auto it = reverse_path.rbegin(); it != reverse_path.rend(); ++it) result.tour.push_back(*it);
    result.tour.push_back(start);
    return result;
}

} // namespace project47
