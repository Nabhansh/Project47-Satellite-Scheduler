#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace project47 {

using IntMatrix = std::vector<std::vector<std::int64_t>>;

inline void validate_square_matrix(const IntMatrix& matrix) {
    if (matrix.empty()) throw std::invalid_argument("Matrix must be non-empty");
    const std::size_t size = matrix.size();
    for (const auto& row : matrix) {
        if (row.size() != size) throw std::invalid_argument("Matrix must be square");
    }
}

inline IntMatrix classical_matrix_multiply(const IntMatrix& left, const IntMatrix& right) {
    validate_square_matrix(left);
    validate_square_matrix(right);
    if (left.size() != right.size()) throw std::invalid_argument("Matrix dimensions must match");
    const std::size_t size = left.size();
    IntMatrix result(size, std::vector<std::int64_t>(size, 0));
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t k = 0; k < size; ++k) {
            for (std::size_t j = 0; j < size; ++j) {
                result[i][j] += left[i][k] * right[k][j];
            }
        }
    }
    return result;
}

namespace detail {
inline IntMatrix add_matrix(const IntMatrix& a, const IntMatrix& b, bool subtract = false) {
    const std::size_t n = a.size();
    IntMatrix result(n, std::vector<std::int64_t>(n, 0));
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            result[i][j] = subtract ? a[i][j] - b[i][j] : a[i][j] + b[i][j];
        }
    }
    return result;
}

inline IntMatrix strassen_power_of_two(const IntMatrix& a, const IntMatrix& b) {
    const std::size_t n = a.size();
    if (n <= 32) return classical_matrix_multiply(a, b);
    const std::size_t half = n / 2;
    const auto quadrant = [half](const IntMatrix& source, std::size_t row, std::size_t col) {
        IntMatrix result(half, std::vector<std::int64_t>(half, 0));
        for (std::size_t i = 0; i < half; ++i) {
            for (std::size_t j = 0; j < half; ++j) result[i][j] = source[row + i][col + j];
        }
        return result;
    };
    const IntMatrix a11 = quadrant(a, 0, 0), a12 = quadrant(a, 0, half);
    const IntMatrix a21 = quadrant(a, half, 0), a22 = quadrant(a, half, half);
    const IntMatrix b11 = quadrant(b, 0, 0), b12 = quadrant(b, 0, half);
    const IntMatrix b21 = quadrant(b, half, 0), b22 = quadrant(b, half, half);

    const IntMatrix m1 = strassen_power_of_two(add_matrix(a11, a22), add_matrix(b11, b22));
    const IntMatrix m2 = strassen_power_of_two(add_matrix(a21, a22), b11);
    const IntMatrix m3 = strassen_power_of_two(a11, add_matrix(b12, b22, true));
    const IntMatrix m4 = strassen_power_of_two(a22, add_matrix(b21, b11, true));
    const IntMatrix m5 = strassen_power_of_two(add_matrix(a11, a12), b22);
    const IntMatrix m6 = strassen_power_of_two(add_matrix(a21, a11, true), add_matrix(b11, b12));
    const IntMatrix m7 = strassen_power_of_two(add_matrix(a12, a22, true), add_matrix(b21, b22));

    const IntMatrix c11 = add_matrix(add_matrix(add_matrix(m1, m4), m5, true), m7);
    const IntMatrix c12 = add_matrix(m3, m5);
    const IntMatrix c21 = add_matrix(m2, m4);
    const IntMatrix c22 = add_matrix(add_matrix(add_matrix(m1, m2, true), m3), m6);
    IntMatrix result(n, std::vector<std::int64_t>(n, 0));
    for (std::size_t i = 0; i < half; ++i) {
        for (std::size_t j = 0; j < half; ++j) {
            result[i][j] = c11[i][j];
            result[i][j + half] = c12[i][j];
            result[i + half][j] = c21[i][j];
            result[i + half][j + half] = c22[i][j];
        }
    }
    return result;
}
} // namespace detail

// Strassen multiplication pads arbitrary square matrices to the next power of
// two, uses a classical cutoff of 32, and crops the result to the input size.
// Time O(n^log2(7)) asymptotically; padding can make it slower for small sizes.
inline IntMatrix strassen_matrix_multiply(const IntMatrix& left, const IntMatrix& right) {
    validate_square_matrix(left);
    validate_square_matrix(right);
    if (left.size() != right.size()) throw std::invalid_argument("Matrix dimensions must match");
    const std::size_t size = left.size();
    std::size_t padded = 1;
    while (padded < size) {
        if (padded > static_cast<std::size_t>(-1) / 2) throw std::length_error("Matrix dimension too large");
        padded *= 2;
    }
    IntMatrix a(padded, std::vector<std::int64_t>(padded, 0));
    IntMatrix b(padded, std::vector<std::int64_t>(padded, 0));
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t j = 0; j < size; ++j) {
            a[i][j] = left[i][j];
            b[i][j] = right[i][j];
        }
    }
    const IntMatrix padded_result = detail::strassen_power_of_two(a, b);
    IntMatrix result(size, std::vector<std::int64_t>(size, 0));
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t j = 0; j < size; ++j) result[i][j] = padded_result[i][j];
    }
    return result;
}

} // namespace project47
