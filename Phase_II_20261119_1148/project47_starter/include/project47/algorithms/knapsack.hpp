#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace project47 {

struct KnapsackItem {
    std::string id;
    std::size_t weight{};
    std::uint64_t value{};
};

struct KnapsackResult {
    std::uint64_t total_value{};
    std::size_t total_weight{};
    std::vector<std::size_t> selected_indices;
};

// Exact 0/1 knapsack using dynamic programming. Runtime and memory: O(n * capacity).
// Equal-value solutions prefer skipping the later item, making reconstruction deterministic.
inline KnapsackResult zero_one_knapsack(const std::vector<KnapsackItem>& items,
                                        std::size_t capacity) {
    std::unordered_set<std::string> ids;
    for (const auto& item : items) {
        if (item.id.empty()) throw std::invalid_argument("Knapsack item ID cannot be empty");
        if (!ids.insert(item.id).second) throw std::invalid_argument("Knapsack item IDs must be unique");
    }

    if (capacity == std::numeric_limits<std::size_t>::max() ||
        items.size() == std::numeric_limits<std::size_t>::max()) {
        throw std::length_error("Knapsack table dimensions overflow");
    }

    const std::size_t columns = capacity + 1;
    if (items.size() + 1 > std::vector<std::uint64_t>{}.max_size() / columns) {
        throw std::length_error("Knapsack table is too large");
    }

    std::vector<std::vector<std::uint64_t>> dp(
        items.size() + 1, std::vector<std::uint64_t>(columns, 0));

    for (std::size_t i = 1; i <= items.size(); ++i) {
        const auto& item = items[i - 1];
        for (std::size_t weight = 0; weight <= capacity; ++weight) {
            dp[i][weight] = dp[i - 1][weight];
            if (item.weight <= weight) {
                const auto previous = dp[i - 1][weight - item.weight];
                if (item.value > std::numeric_limits<std::uint64_t>::max() - previous) {
                    throw std::overflow_error("Knapsack value total overflows uint64_t");
                }
                const auto candidate = previous + item.value;
                if (candidate > dp[i][weight]) dp[i][weight] = candidate;
            }
        }
    }

    KnapsackResult result;
    result.total_value = dp[items.size()][capacity];
    std::size_t remaining = capacity;
    for (std::size_t i = items.size(); i > 0; --i) {
        if (dp[i][remaining] != dp[i - 1][remaining]) {
            const auto index = i - 1;
            result.selected_indices.push_back(index);
            result.total_weight += items[index].weight;
            remaining -= items[index].weight;
        }
    }
    // Reconstruction walks backwards; return selected indices in input order.
    for (std::size_t left = 0, right = result.selected_indices.size(); left < right / 2; ++left) {
        const auto temp = result.selected_indices[left];
        result.selected_indices[left] = result.selected_indices[right - left - 1];
        result.selected_indices[right - left - 1] = temp;
    }
    return result;
}

} // namespace project47
