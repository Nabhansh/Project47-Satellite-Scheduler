#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace project47 {

// In-place three-way quick sort. Median-of-three pivot selection handles
// already-sorted inputs well; three-way partitioning handles duplicate keys.
// Average time: O(n log n). Worst-case time: O(n^2). Auxiliary stack: O(log n).
template <typename T, typename Compare = std::less<T>>
void quick_sort(std::vector<T>& values, Compare comp = Compare{}) {
    if (values.size() < 2) {
        return;
    }

    const auto sort_range = [&](auto&& self, std::size_t first, std::size_t last) -> void {
        while (last - first > 1) {
            const std::size_t middle = first + (last - first) / 2;
            const std::size_t final_index = last - 1;

            // Order the first, middle, and final values; the middle is the pivot.
            if (comp(values[middle], values[first])) {
                std::swap(values[middle], values[first]);
            }
            if (comp(values[final_index], values[middle])) {
                std::swap(values[final_index], values[middle]);
            }
            if (comp(values[middle], values[first])) {
                std::swap(values[middle], values[first]);
            }
            const T pivot = values[middle];

            // Dutch-national-flag partition: [first, less_end) < pivot,
            // [less_end, scan) equivalent, [greater_begin, last) > pivot.
            std::size_t less_end = first;
            std::size_t scan = first;
            std::size_t greater_begin = last;

            while (scan < greater_begin) {
                if (comp(values[scan], pivot)) {
                    std::swap(values[less_end++], values[scan++]);
                } else if (comp(pivot, values[scan])) {
                    std::swap(values[scan], values[--greater_begin]);
                } else {
                    ++scan;
                }
            }

            // Recurse only into the smaller side. Iterating over the larger
            // side bounds recursion depth to O(log n), even for poor pivots.
            const std::size_t left_size = less_end - first;
            const std::size_t right_size = last - greater_begin;
            if (left_size < right_size) {
                self(self, first, less_end);
                first = greater_begin;
            } else {
                self(self, greater_begin, last);
                last = less_end;
            }
        }
    };

    sort_range(sort_range, 0, values.size());
}

} // namespace project47
