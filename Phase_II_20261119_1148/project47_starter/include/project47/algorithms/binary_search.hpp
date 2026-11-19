#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

namespace project47 {
namespace detail {

template <typename T, typename Compare>
std::size_t lower_bound_recursive(const std::vector<T>& values,
                                  const T& target,
                                  std::size_t first,
                                  std::size_t last,
                                  Compare comp) {
    if (first == last) {
        return first;
    }

    const std::size_t middle = first + (last - first) / 2;
    if (comp(values[middle], target)) {
        return lower_bound_recursive(values, target, middle + 1, last, comp);
    }
    return lower_bound_recursive(values, target, first, middle, comp);
}

template <typename T, typename Compare>
bool equivalent(const T& left, const T& right, Compare comp) {
    return !comp(left, right) && !comp(right, left);
}

} // namespace detail

// Returns the first matching index, or std::nullopt when the target is absent.
// The input must already be sorted according to Compare. Time: O(log n).
template <typename T, typename Compare = std::less<T>>
std::optional<std::size_t> binary_search_iterative(const std::vector<T>& values,
                                                   const T& target,
                                                   Compare comp = Compare{}) {
    std::size_t first = 0;
    std::size_t last = values.size();

    while (first < last) {
        const std::size_t middle = first + (last - first) / 2;
        if (comp(values[middle], target)) {
            first = middle + 1;
        } else {
            last = middle;
        }
    }

    if (first < values.size() && detail::equivalent(values[first], target, comp)) {
        return first;
    }
    return std::nullopt;
}

// Recursive counterpart with the same first-match behavior and complexity.
template <typename T, typename Compare = std::less<T>>
std::optional<std::size_t> binary_search_recursive(const std::vector<T>& values,
                                                   const T& target,
                                                   Compare comp = Compare{}) {
    const std::size_t index = detail::lower_bound_recursive(
        values, target, 0, values.size(), comp);

    if (index < values.size() && detail::equivalent(values[index], target, comp)) {
        return index;
    }
    return std::nullopt;
}

} // namespace project47
