#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace project47 {

enum class BranchAndBoundStrategy { FIFO, LIFO, LeastCost };

inline const char* to_string(BranchAndBoundStrategy strategy) noexcept {
    switch (strategy) {
        case BranchAndBoundStrategy::FIFO: return "FIFO";
        case BranchAndBoundStrategy::LIFO: return "LIFO";
        case BranchAndBoundStrategy::LeastCost: return "least-cost";
    }
    return "unknown";
}

struct BnBItem {
    std::string id;
    std::size_t weight{};
    std::uint64_t value{};
};

struct BranchAndBoundResult {
    std::uint64_t total_value{};
    std::size_t total_weight{};
    std::vector<std::size_t> selected_indices;
    std::uint64_t nodes_expanded{};
    std::uint64_t nodes_generated{};
};

namespace detail {
struct DensityItem {
    std::size_t original_index{};
    std::size_t weight{};
    std::uint64_t value{};
};
struct BnBNode {
    std::size_t level{};
    std::size_t weight{};
    std::uint64_t value{};
    long double upper_bound{};
    std::vector<std::size_t> selected_indices;
};

inline long double fractional_upper_bound(const BnBNode& node,
                                          const std::vector<DensityItem>& items,
                                          std::size_t capacity) {
    if (node.weight > capacity) return -1.0L;
    long double bound = static_cast<long double>(node.value);
    std::size_t remaining = capacity - node.weight;
    for (std::size_t i = node.level; i < items.size(); ++i) {
        const auto& item = items[i];
        if (item.weight == 0) {
            bound += static_cast<long double>(item.value);
        } else if (item.weight <= remaining) {
            remaining -= item.weight;
            bound += static_cast<long double>(item.value);
        } else {
            bound += static_cast<long double>(item.value) *
                     static_cast<long double>(remaining) /
                     static_cast<long double>(item.weight);
            break;
        }
    }
    return bound;
}

inline void validate_bnb_items(const std::vector<BnBItem>& items) {
    std::unordered_set<std::string> identifiers;
    for (const auto& item : items) {
        if (item.id.empty()) throw std::invalid_argument("Branch-and-bound item ID cannot be empty");
        if (!identifiers.insert(item.id).second) {
            throw std::invalid_argument("Branch-and-bound item IDs must be unique");
        }
    }
}

inline std::vector<DensityItem> density_order(const std::vector<BnBItem>& items) {
    std::vector<DensityItem> ordered;
    ordered.reserve(items.size());
    for (std::size_t i = 0; i < items.size(); ++i) {
        ordered.push_back({i, items[i].weight, items[i].value});
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const DensityItem& lhs,
                                                        const DensityItem& rhs) {
        const long double lhs_ratio = lhs.weight == 0
            ? (lhs.value == 0 ? 0.0L : std::numeric_limits<long double>::infinity())
            : static_cast<long double>(lhs.value) / static_cast<long double>(lhs.weight);
        const long double rhs_ratio = rhs.weight == 0
            ? (rhs.value == 0 ? 0.0L : std::numeric_limits<long double>::infinity())
            : static_cast<long double>(rhs.value) / static_cast<long double>(rhs.weight);
        if (lhs_ratio != rhs_ratio) return lhs_ratio > rhs_ratio;
        return lhs.original_index < rhs.original_index;
    });
    return ordered;
}

inline BnBNode make_child(const BnBNode& parent, const DensityItem& item,
                          std::size_t next_level, bool include,
                          const std::vector<DensityItem>& ordered,
                          std::size_t capacity) {
    BnBNode child;
    child.level = next_level;
    child.weight = parent.weight;
    child.value = parent.value;
    child.selected_indices = parent.selected_indices;
    if (include) {
        if (item.weight > capacity - parent.weight) {
            child.upper_bound = -1.0L;
            return child;
        }
        child.weight += item.weight;
        if (item.value > std::numeric_limits<std::uint64_t>::max() - child.value) {
            throw std::overflow_error("Branch-and-bound value total overflows uint64_t");
        }
        child.value += item.value;
        child.selected_indices.push_back(item.original_index);
    }
    child.upper_bound = fractional_upper_bound(child, ordered, capacity);
    return child;
}

inline void accept_node(const BnBNode& node, BranchAndBoundResult& result) {
    if (node.value > result.total_value) {
        result.total_value = node.value;
        result.total_weight = node.weight;
        result.selected_indices = node.selected_indices;
    }
}

inline void increment(std::uint64_t& counter, const char* label) {
    if (counter == std::numeric_limits<std::uint64_t>::max()) {
        throw std::overflow_error(std::string("Branch-and-bound ") + label + " counter overflow");
    }
    ++counter;
}
} // namespace detail

// Exact 0/1 knapsack B&B. Least-cost uses cost = -upper_bound, hence the
// smallest cost expands the most promising maximum-value node first.
// Worst-case time is O(2^n); search discipline changes order, not correctness.
inline BranchAndBoundResult branch_and_bound_knapsack(
    const std::vector<BnBItem>& items, std::size_t capacity,
    BranchAndBoundStrategy strategy) {
    detail::validate_bnb_items(items);
    switch (strategy) {
        case BranchAndBoundStrategy::FIFO:
        case BranchAndBoundStrategy::LIFO:
        case BranchAndBoundStrategy::LeastCost:
            break;
        default:
            throw std::invalid_argument("Unknown branch-and-bound strategy");
    }
    const auto ordered = detail::density_order(items);
    BranchAndBoundResult result;
    detail::BnBNode root;
    root.upper_bound = detail::fractional_upper_bound(root, ordered, capacity);
    detail::increment(result.nodes_generated, "generated-node");

    std::deque<detail::BnBNode> fifo;
    std::vector<detail::BnBNode> lifo;
    struct LeastCostCompare {
        bool operator()(const detail::BnBNode& lhs, const detail::BnBNode& rhs) const {
            return lhs.upper_bound < rhs.upper_bound; // cost is -bound; min-cost at top
        }
    };
    std::priority_queue<detail::BnBNode, std::vector<detail::BnBNode>, LeastCostCompare> least_cost;

    const auto push = [&](detail::BnBNode node) {
        if (node.upper_bound < 0.0L ||
            node.upper_bound <= static_cast<long double>(result.total_value)) return;
        detail::increment(result.nodes_generated, "generated-node");
        switch (strategy) {
            case BranchAndBoundStrategy::FIFO: fifo.push_back(std::move(node)); break;
            case BranchAndBoundStrategy::LIFO: lifo.push_back(std::move(node)); break;
            case BranchAndBoundStrategy::LeastCost: least_cost.push(std::move(node)); break;
        }
    };
    switch (strategy) {
        case BranchAndBoundStrategy::FIFO: fifo.push_back(root); break;
        case BranchAndBoundStrategy::LIFO: lifo.push_back(root); break;
        case BranchAndBoundStrategy::LeastCost: least_cost.push(root); break;
    }
    const auto has_work = [&]() {
        switch (strategy) {
            case BranchAndBoundStrategy::FIFO: return !fifo.empty();
            case BranchAndBoundStrategy::LIFO: return !lifo.empty();
            case BranchAndBoundStrategy::LeastCost: return !least_cost.empty();
        }
        return false;
    };

    while (has_work()) {
        detail::BnBNode current;
        switch (strategy) {
            case BranchAndBoundStrategy::FIFO:
                current = std::move(fifo.front()); fifo.pop_front(); break;
            case BranchAndBoundStrategy::LIFO:
                current = std::move(lifo.back()); lifo.pop_back(); break;
            case BranchAndBoundStrategy::LeastCost:
                current = least_cost.top(); least_cost.pop(); break;
        }
        if (current.upper_bound <= static_cast<long double>(result.total_value)) continue;
        detail::increment(result.nodes_expanded, "expanded-node");
        detail::accept_node(current, result);
        if (current.level >= ordered.size()) continue;

        const auto& item = ordered[current.level];
        auto include = detail::make_child(current, item, current.level + 1,
                                          true, ordered, capacity);
        auto exclude = detail::make_child(current, item, current.level + 1,
                                          false, ordered, capacity);
        detail::accept_node(include, result);
        detail::accept_node(exclude, result);
        push(std::move(include));
        push(std::move(exclude));
    }
    std::sort(result.selected_indices.begin(), result.selected_indices.end());
    return result;
}

} // namespace project47
