#pragma once

#include <cstddef>
#include <functional>
#include <vector>

namespace project47 {

// Stable O(n log n) merge sort. The comparator follows the standard convention:
// comp(a, b) is true when a should appear before b.
template <typename T, typename Compare = std::less<T>>
void merge_sort(std::vector<T>& values, Compare comp = Compare{}) {
    if (values.size() < 2) {
        return;
    }

    std::vector<T> buffer(values.size());

    const auto sort_range = [&](auto&& self, std::size_t first, std::size_t last) -> void {
        if (last - first < 2) {
            return;
        }

        const std::size_t middle = first + (last - first) / 2;
        self(self, first, middle);
        self(self, middle, last);

        std::size_t left = first;
        std::size_t right = middle;
        std::size_t out = first;

        while (left < middle && right < last) {
            // Choosing the left item on equality preserves stability.
            if (comp(values[right], values[left])) {
                buffer[out++] = values[right++];
            } else {
                buffer[out++] = values[left++];
            }
        }
        while (left < middle) {
            buffer[out++] = values[left++];
        }
        while (right < last) {
            buffer[out++] = values[right++];
        }

        for (std::size_t i = first; i < last; ++i) {
            values[i] = buffer[i];
        }
    };

    sort_range(sort_range, 0, values.size());
}

} // namespace project47
