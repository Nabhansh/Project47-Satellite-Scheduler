#pragma once

#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace project47 {

struct Congruence {
    std::int64_t residue{};
    std::int64_t modulus{};
};

struct ChineseRemainderResult {
    std::int64_t residue{};
    std::int64_t modulus{};
};

namespace crt_detail {

inline std::int64_t normalize(std::int64_t value, std::int64_t modulus) {
    const std::int64_t remainder = value % modulus;
    return remainder < 0 ? remainder + modulus : remainder;
}

inline std::int64_t add_mod(std::int64_t left, std::int64_t right, std::int64_t modulus) {
    // Inputs are normalized. This form avoids overflowing left + right.
    return left >= modulus - right ? left - (modulus - right) : left + right;
}

inline std::int64_t multiply_mod(std::int64_t left, std::int64_t right,
                                std::int64_t modulus) {
    std::int64_t result = 0;
    left = normalize(left, modulus);
    right = normalize(right, modulus);
    while (right > 0) {
        if ((right & 1) != 0) result = add_mod(result, left, modulus);
        right >>= 1;
        if (right > 0) left = add_mod(left, left, modulus);
    }
    return result;
}

inline std::int64_t checked_multiply(std::int64_t left, std::int64_t right) {
    if (left == 0 || right == 0) return 0;
    constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    if (left == -1 && right == minimum) throw std::overflow_error("CRT arithmetic overflow");
    if (right == -1 && left == minimum) throw std::overflow_error("CRT arithmetic overflow");
    if (left > 0) {
        if (right > 0 && left > maximum / right) throw std::overflow_error("CRT modulus overflow");
        if (right < 0 && right < minimum / left) throw std::overflow_error("CRT arithmetic overflow");
    } else {
        if (right > 0 && left < minimum / right) throw std::overflow_error("CRT arithmetic overflow");
        if (right < 0 && left < maximum / right) throw std::overflow_error("CRT arithmetic overflow");
    }
    return left * right;
}

inline std::int64_t checked_subtract(std::int64_t left, std::int64_t right) {
    if (right == std::numeric_limits<std::int64_t>::min()) {
        if (left >= 0) throw std::overflow_error("CRT arithmetic overflow");
        return left - right;
    }
    return left + (-right);
}

inline std::int64_t modular_inverse(std::int64_t value, std::int64_t modulus) {
    std::int64_t old_remainder = modulus;
    std::int64_t remainder = normalize(value, modulus);
    std::int64_t old_coefficient = 0;
    std::int64_t coefficient = 1;

    while (remainder != 0) {
        const std::int64_t quotient = old_remainder / remainder;
        const std::int64_t next_remainder = old_remainder % remainder;
        const std::int64_t product = checked_multiply(quotient, coefficient);
        const std::int64_t next_coefficient = checked_subtract(old_coefficient, product);
        old_remainder = remainder;
        remainder = next_remainder;
        old_coefficient = coefficient;
        coefficient = next_coefficient;
    }
    if (old_remainder != 1) throw std::invalid_argument("CRT moduli must be pairwise coprime");
    return normalize(old_coefficient, modulus);
}

} // namespace crt_detail

// Solves pairwise-coprime congruences. The combined modulus and result must fit int64_t.
inline ChineseRemainderResult chinese_remainder_theorem(const std::vector<Congruence>& equations) {
    if (equations.empty()) throw std::invalid_argument("CRT requires at least one congruence");
    for (const Congruence& equation : equations) {
        if (equation.modulus <= 1) {
            throw std::invalid_argument("CRT moduli must be greater than one");
        }
    }

    ChineseRemainderResult result{0, 1};
    for (const Congruence& equation : equations) {
        const std::int64_t modulus = equation.modulus;
        if (std::gcd(result.modulus, modulus) != 1) {
            throw std::invalid_argument("CRT moduli must be pairwise coprime");
        }
        const std::int64_t target = crt_detail::normalize(equation.residue, modulus);
        const std::int64_t current = result.residue % modulus;
        const std::int64_t difference = target >= current
            ? target - current : modulus - (current - target);
        const std::int64_t inverse = crt_detail::modular_inverse(result.modulus % modulus, modulus);
        const std::int64_t multiplier = crt_detail::multiply_mod(difference, inverse, modulus);
        const std::int64_t increment = crt_detail::checked_multiply(result.modulus, multiplier);
        if (result.residue > std::numeric_limits<std::int64_t>::max() - increment) {
            throw std::overflow_error("CRT result overflow");
        }
        const std::int64_t combined_modulus =
            crt_detail::checked_multiply(result.modulus, modulus);
        result.residue += increment;
        result.modulus = combined_modulus;
    }
    return result;
}

} // namespace project47
