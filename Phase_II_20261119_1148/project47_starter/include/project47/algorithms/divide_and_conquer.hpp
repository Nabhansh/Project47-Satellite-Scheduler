#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace project47 {

struct MaximumSubarrayResult {
    std::int64_t sum{};
    std::size_t begin{}; // Inclusive.
    std::size_t end{};   // Exclusive.
};

namespace detail {
inline MaximumSubarrayResult max_crossing_subarray(
    const std::vector<std::int64_t>& values, std::size_t first,
    std::size_t middle, std::size_t last) {
    std::int64_t running = 0;
    std::int64_t best_left = std::numeric_limits<std::int64_t>::lowest();
    std::size_t best_left_index = middle;
    for (std::size_t i = middle; i > first; --i) {
        running += values[i - 1];
        if (running > best_left) {
            best_left = running;
            best_left_index = i - 1;
        }
    }

    running = 0;
    std::int64_t best_right = std::numeric_limits<std::int64_t>::lowest();
    std::size_t best_right_index = middle + 1;
    for (std::size_t i = middle; i < last; ++i) {
        running += values[i];
        if (running > best_right) {
            best_right = running;
            best_right_index = i + 1;
        }
    }
    return {best_left + best_right, best_left_index, best_right_index};
}

inline MaximumSubarrayResult maximum_subarray_range(
    const std::vector<std::int64_t>& values, std::size_t first, std::size_t last) {
    if (last - first == 1) return {values[first], first, last};
    const std::size_t middle = first + (last - first) / 2;
    const auto left = maximum_subarray_range(values, first, middle);
    const auto right = maximum_subarray_range(values, middle, last);
    const auto crossing = max_crossing_subarray(values, first, middle, last);
    if (left.sum >= right.sum && left.sum >= crossing.sum) return left;
    if (right.sum >= left.sum && right.sum >= crossing.sum) return right;
    return crossing;
}
} // namespace detail

// Divide-and-conquer maximum contiguous subarray. Empty input is rejected.
// Time O(n log n), recursion space O(log n).
inline MaximumSubarrayResult maximum_subarray_divide_conquer(
    const std::vector<std::int64_t>& values) {
    if (values.empty()) throw std::invalid_argument("Maximum subarray requires non-empty input");
    return detail::maximum_subarray_range(values, 0, values.size());
}

// Exponentiation by squaring. Negative exponents return reciprocals.
// Time O(log |exponent|), O(1) auxiliary space.
inline double fast_power(double base, int exponent) {
    if (!std::isfinite(base)) throw std::invalid_argument("Power base must be finite");
    if (base == 0.0 && exponent < 0) {
        throw std::domain_error("Zero cannot be raised to a negative exponent");
    }
    std::uint64_t power = exponent < 0
        ? static_cast<std::uint64_t>(-(static_cast<std::int64_t>(exponent)))
        : static_cast<std::uint64_t>(exponent);
    double result = 1.0;
    double factor = base;
    while (power > 0) {
        if ((power & 1U) != 0U) result *= factor;
        power >>= 1U;
        if (power > 0) factor *= factor;
    }
    return exponent < 0 ? 1.0 / result : result;
}

} // namespace project47
