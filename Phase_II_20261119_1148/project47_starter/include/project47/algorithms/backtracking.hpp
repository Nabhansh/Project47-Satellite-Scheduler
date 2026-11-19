#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace project47 {

struct NQueensResult {
    std::vector<std::vector<std::size_t>> solutions;
    std::uint64_t nodes_explored{};
    bool truncated{};
};

// Placements are column indices, one per row. Enumeration is capped at 15
// columns to keep exhaustive search and its memory use bounded in this CLI.
inline NQueensResult solve_n_queens(std::size_t board_size,
                                    std::size_t max_solutions = 1000) {
    if (board_size > 15) {
        throw std::invalid_argument("N-Queens board size is limited to 15 for exhaustive enumeration");
    }

    NQueensResult result;
    std::vector<std::size_t> placement(board_size, 0);
    std::vector<bool> occupied_columns(board_size, false);
    std::vector<bool> occupied_descending(2 * board_size, false);
    std::vector<bool> occupied_ascending(2 * board_size, false);

    const auto search = [&](const auto& self, std::size_t row) -> void {
        if (result.nodes_explored == std::numeric_limits<std::uint64_t>::max()) {
            throw std::overflow_error("N-Queens node counter overflow");
        }
        ++result.nodes_explored;
        if (row == board_size) {
            if (result.solutions.size() < max_solutions) result.solutions.push_back(placement);
            else result.truncated = true;
            return;
        }
        if (result.truncated) return;

        for (std::size_t column = 0; column < board_size; ++column) {
            const std::size_t descending = row + (board_size - 1 - column);
            const std::size_t ascending = row + column;
            if (occupied_columns[column] || occupied_descending[descending] ||
                occupied_ascending[ascending]) continue;
            placement[row] = column;
            occupied_columns[column] = true;
            occupied_descending[descending] = true;
            occupied_ascending[ascending] = true;
            self(self, row + 1);
            occupied_columns[column] = false;
            occupied_descending[descending] = false;
            occupied_ascending[ascending] = false;
            if (result.truncated) return;
        }
    };

    search(search, 0);
    return result;
}

struct SubsetSumResult {
    // Solutions contain input indices, preserving duplicate-valued choices.
    std::vector<std::vector<std::size_t>> selected_indices;
    std::uint64_t nodes_explored{};
    bool truncated{};
};

inline std::int64_t checked_subset_sum_add(std::int64_t lhs, std::int64_t rhs) {
    if ((rhs > 0 && lhs > std::numeric_limits<std::int64_t>::max() - rhs) ||
        (rhs < 0 && lhs < std::numeric_limits<std::int64_t>::min() - rhs)) {
        throw std::overflow_error("Subset-sum partial total overflows int64_t");
    }
    return lhs + rhs;
}

// Enumerates exact subsets, including negative values. The output cap bounds
// storage only; the worst-case search still visits O(2^n) states.
inline SubsetSumResult subset_sum_backtracking(const std::vector<std::int64_t>& values,
                                               std::int64_t target,
                                               std::size_t max_solutions = 1000) {
    SubsetSumResult result;
    std::vector<std::size_t> chosen;
    const auto search = [&](const auto& self, std::size_t index,
                            std::int64_t current_sum) -> void {
        if (result.nodes_explored == std::numeric_limits<std::uint64_t>::max()) {
            throw std::overflow_error("Subset-sum node counter overflow");
        }
        ++result.nodes_explored;
        if (index == values.size()) {
            if (current_sum == target) {
                if (result.selected_indices.size() < max_solutions) {
                    result.selected_indices.push_back(chosen);
                } else {
                    result.truncated = true;
                }
            }
            return;
        }
        chosen.push_back(index);
        self(self, index + 1, checked_subset_sum_add(current_sum, values[index]));
        chosen.pop_back();
        self(self, index + 1, current_sum);
    };
    search(search, 0, 0);
    return result;
}

} // namespace project47
