#pragma once

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

namespace project47 {

inline void validate_celebrity_matrix(const std::vector<std::vector<bool>>& knows) {
    const std::size_t count = knows.size();
    for (const auto& row : knows) {
        if (row.size() != count) throw std::invalid_argument("Celebrity matrix must be square");
    }
}

// Divide-and-conquer celebrity candidate reduction. Each half returns at most
// one candidate; one cross-half relation resolves the two candidates. A final
// O(n) verification is required. Recurrence: T(n)=2T(n/2)+O(1)=O(n).
// Auxiliary recursion space: O(log n); the input relation matrix is O(n^2).
inline std::optional<std::size_t> find_celebrity_divide_conquer(
    const std::vector<std::vector<bool>>& knows) {
    const std::size_t count = knows.size();
    validate_celebrity_matrix(knows);
    if (count == 0) return std::nullopt;

    const auto reduce = [&](auto&& self, std::size_t first,
                            std::size_t last) -> std::optional<std::size_t> {
        if (last - first == 1) return first;
        const std::size_t middle = first + (last - first) / 2;
        const auto left = self(self, first, middle);
        const auto right = self(self, middle, last);
        if (!left) return right;
        if (!right) return left;

        const bool left_knows_right = knows[*left][*right];
        const bool right_knows_left = knows[*right][*left];
        if (!left_knows_right && right_knows_left) return left;
        if (left_knows_right && !right_knows_left) return right;
        return std::nullopt;
    };

    const auto candidate = reduce(reduce, 0, count);
    if (!candidate) return std::nullopt;
    for (std::size_t person = 0; person < count; ++person) {
        if (person == *candidate) continue;
        if (knows[*candidate][person] || !knows[person][*candidate]) return std::nullopt;
    }
    return candidate;
}

// Linear-time elimination alternative. Uses O(n) relation checks and O(1)
// auxiliary space after the relation matrix has been supplied.
inline std::optional<std::size_t> find_celebrity(
    const std::vector<std::vector<bool>>& knows) {
    const std::size_t count = knows.size();
    validate_celebrity_matrix(knows);
    if (count == 0) return std::nullopt;

    std::size_t candidate = 0;
    for (std::size_t person = 1; person < count; ++person) {
        if (knows[candidate][person]) candidate = person;
    }
    for (std::size_t person = 0; person < count; ++person) {
        if (person == candidate) continue;
        if (knows[candidate][person] || !knows[person][candidate]) return std::nullopt;
    }
    return candidate;
}

} // namespace project47
