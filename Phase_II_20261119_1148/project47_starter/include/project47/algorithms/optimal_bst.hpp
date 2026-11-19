#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct OptimalBstResult {
    std::uint64_t expected_cost{};
    // root[i][j] contains the zero-based key index chosen for the interval [i, j].
    // Empty intervals are not queried.
    std::vector<std::vector<std::size_t>> root;
    std::size_t key_count{};
};

inline std::uint64_t checked_frequency_sum(std::uint64_t a, std::uint64_t b) {
    if (b > std::numeric_limits<std::uint64_t>::max() - a) {
        throw std::overflow_error("Optimal BST frequency or cost overflows uint64_t");
    }
    return a + b;
}

// Optimal Binary Search Tree with successful p[0..n-1] and unsuccessful q[0..n].
// Runtime: O(n^3), memory: O(n^2). Keys must be sorted and unique.
inline OptimalBstResult optimal_binary_search_tree(
    const std::vector<std::string>& keys,
    const std::vector<std::uint64_t>& successful_frequency,
    const std::vector<std::uint64_t>& unsuccessful_frequency) {
    const std::size_t n = keys.size();
    if (successful_frequency.size() != n || unsuccessful_frequency.size() != n + 1) {
        throw std::invalid_argument("Optimal BST needs n successful and n+1 unsuccessful frequencies");
    }
    for (std::size_t i = 0; i < n; ++i) {
        if (keys[i].empty()) throw std::invalid_argument("Optimal BST keys cannot be empty");
        if (i > 0 && !(keys[i - 1] < keys[i])) {
            throw std::invalid_argument("Optimal BST keys must be sorted and unique");
        }
    }

    // e[i][j] and w[i][j] use the classic 1-based key indexing; e[i][i-1] is q[i-1].
    std::vector<std::vector<std::uint64_t>> expected(
        n + 2, std::vector<std::uint64_t>(n + 1, 0));
    std::vector<std::vector<std::uint64_t>> weight(
        n + 2, std::vector<std::uint64_t>(n + 1, 0));
    OptimalBstResult result;
    result.key_count = n;
    result.root.assign(n + 2, std::vector<std::size_t>(n + 1, n));

    for (std::size_t i = 1; i <= n + 1; ++i) {
        expected[i][i - 1] = unsuccessful_frequency[i - 1];
        weight[i][i - 1] = unsuccessful_frequency[i - 1];
    }

    for (std::size_t length = 1; length <= n; ++length) {
        for (std::size_t i = 1; i + length - 1 <= n; ++i) {
            const std::size_t j = i + length - 1;
            weight[i][j] = checked_frequency_sum(
                checked_frequency_sum(weight[i][j - 1], successful_frequency[j - 1]),
                unsuccessful_frequency[j]);
            expected[i][j] = std::numeric_limits<std::uint64_t>::max();
            for (std::size_t r = i; r <= j; ++r) {
                const auto children = checked_frequency_sum(expected[i][r - 1], expected[r + 1][j]);
                const auto candidate = checked_frequency_sum(children, weight[i][j]);
                if (candidate < expected[i][j]) {
                    expected[i][j] = candidate;
                    result.root[i][j] = r - 1;
                }
            }
        }
    }

    result.expected_cost = n == 0 ? unsuccessful_frequency[0] : expected[1][n];
    return result;
}

} // namespace project47
