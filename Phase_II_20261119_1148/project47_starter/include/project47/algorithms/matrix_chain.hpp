#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct MatrixChainResult {
    std::uint64_t minimum_multiplications{};
    std::string parenthesization;
    std::vector<std::vector<std::uint64_t>> costs;
    // split[i][j] is the zero-based split after matrix i..split, for 1-based i,j.
    std::vector<std::vector<std::size_t>> split;
};

inline std::uint64_t checked_matrix_product(std::uint64_t a, std::uint64_t b) {
    if (a != 0 && b > std::numeric_limits<std::uint64_t>::max() / a) {
        throw std::overflow_error("Matrix-chain multiplication cost overflows uint64_t");
    }
    return a * b;
}

inline std::uint64_t checked_matrix_sum(std::uint64_t a, std::uint64_t b) {
    if (b > std::numeric_limits<std::uint64_t>::max() - a) {
        throw std::overflow_error("Matrix-chain multiplication cost overflows uint64_t");
    }
    return a + b;
}

// Matrix Chain Multiplication. Runtime: O(n^3), memory: O(n^2).
inline MatrixChainResult matrix_chain_order(const std::vector<std::size_t>& dimensions) {
    if (dimensions.size() < 2) {
        throw std::invalid_argument("Matrix chain needs at least one matrix (two dimensions)");
    }
    for (const auto dimension : dimensions) {
        if (dimension == 0) throw std::invalid_argument("Matrix dimensions must be positive");
        if (dimension > std::numeric_limits<std::uint64_t>::max()) {
            throw std::overflow_error("Matrix dimension exceeds supported cost range");
        }
    }

    const std::size_t n = dimensions.size() - 1;
    MatrixChainResult result;
    result.costs.assign(n + 1, std::vector<std::uint64_t>(n + 1, 0));
    result.split.assign(n + 1, std::vector<std::size_t>(n + 1, 0));

    for (std::size_t length = 2; length <= n; ++length) {
        for (std::size_t i = 1; i + length - 1 <= n; ++i) {
            const std::size_t j = i + length - 1;
            result.costs[i][j] = std::numeric_limits<std::uint64_t>::max();
            for (std::size_t k = i; k < j; ++k) {
                auto product = checked_matrix_product(
                    static_cast<std::uint64_t>(dimensions[i - 1]),
                    static_cast<std::uint64_t>(dimensions[k]));
                product = checked_matrix_product(product,
                    static_cast<std::uint64_t>(dimensions[j]));
                auto candidate = checked_matrix_sum(result.costs[i][k], result.costs[k + 1][j]);
                candidate = checked_matrix_sum(candidate, product);
                if (candidate < result.costs[i][j]) {
                    result.costs[i][j] = candidate;
                    result.split[i][j] = k;
                }
            }
        }
    }

    result.minimum_multiplications = result.costs[1][n];
    const auto build = [&](const auto& self, std::size_t i, std::size_t j) -> std::string {
        if (i == j) return "A" + std::to_string(i);
        const auto k = result.split[i][j];
        return "(" + self(self, i, k) + " x " + self(self, k + 1, j) + ")";
    };
    result.parenthesization = build(build, 1, n);
    return result;
}

} // namespace project47
