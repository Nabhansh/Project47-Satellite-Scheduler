#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace project47 {

inline std::vector<std::size_t> naive_string_search(const std::string& text,
                                                     const std::string& pattern) {
    std::vector<std::size_t> matches;
    if (pattern.empty()) {
        matches.reserve(text.size() + 1);
        for (std::size_t i = 0; i <= text.size(); ++i) matches.push_back(i);
        return matches;
    }
    if (pattern.size() > text.size()) return matches;
    for (std::size_t start = 0; start <= text.size() - pattern.size(); ++start) {
        std::size_t offset = 0;
        while (offset < pattern.size() && text[start + offset] == pattern[offset]) ++offset;
        if (offset == pattern.size()) matches.push_back(start);
    }
    return matches;
}

inline std::vector<std::size_t> rabin_karp_search(const std::string& text,
                                                  const std::string& pattern) {
    constexpr std::uint64_t base = 257;
    constexpr std::uint64_t modulus = 1'000'000'007;
    std::vector<std::size_t> matches;
    if (pattern.empty()) {
        matches.reserve(text.size() + 1);
        for (std::size_t i = 0; i <= text.size(); ++i) matches.push_back(i);
        return matches;
    }
    if (pattern.size() > text.size()) return matches;

    const auto symbol = [](char value) {
        return static_cast<std::uint64_t>(static_cast<unsigned char>(value)) + 1U;
    };
    std::uint64_t leading_power = 1;
    for (std::size_t i = 1; i < pattern.size(); ++i) {
        leading_power = (leading_power * base) % modulus;
    }

    std::uint64_t pattern_hash = 0;
    std::uint64_t window_hash = 0;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        pattern_hash = (pattern_hash * base + symbol(pattern[i])) % modulus;
        window_hash = (window_hash * base + symbol(text[i])) % modulus;
    }

    for (std::size_t start = 0; start <= text.size() - pattern.size(); ++start) {
        if (pattern_hash == window_hash &&
            text.compare(start, pattern.size(), pattern) == 0) {
            matches.push_back(start);
        }
        if (start < text.size() - pattern.size()) {
            const std::uint64_t leading = (symbol(text[start]) * leading_power) % modulus;
            window_hash = (window_hash + modulus - leading) % modulus;
            window_hash = (window_hash * base + symbol(text[start + pattern.size()])) % modulus;
        }
    }
    return matches;
}

inline std::vector<std::size_t> prefix_function(const std::string& pattern) {
    std::vector<std::size_t> prefix(pattern.size(), 0);
    for (std::size_t i = 1; i < pattern.size(); ++i) {
        std::size_t matched = prefix[i - 1];
        while (matched > 0 && pattern[i] != pattern[matched]) matched = prefix[matched - 1];
        if (pattern[i] == pattern[matched]) ++matched;
        prefix[i] = matched;
    }
    return prefix;
}

inline std::vector<std::size_t> kmp_search(const std::string& text,
                                           const std::string& pattern) {
    std::vector<std::size_t> matches;
    if (pattern.empty()) {
        matches.reserve(text.size() + 1);
        for (std::size_t i = 0; i <= text.size(); ++i) matches.push_back(i);
        return matches;
    }
    if (pattern.size() > text.size()) return matches;

    const auto prefix = prefix_function(pattern);
    std::size_t matched = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        while (matched > 0 && text[i] != pattern[matched]) matched = prefix[matched - 1];
        if (text[i] == pattern[matched]) ++matched;
        if (matched == pattern.size()) {
            matches.push_back(i + 1 - pattern.size());
            matched = prefix[matched - 1];
        }
    }
    return matches;
}

// Finite-automaton matcher over the complete byte alphabet (0..255).
// The transition table uses O(256 * m) space for a pattern of length m.
inline std::vector<std::size_t> finite_automaton_search(const std::string& text,
                                                        const std::string& pattern) {
    std::vector<std::size_t> matches;
    if (pattern.empty()) {
        matches.reserve(text.size() + 1);
        for (std::size_t i = 0; i <= text.size(); ++i) matches.push_back(i);
        return matches;
    }
    if (pattern.size() > text.size()) return matches;

    const auto prefix = prefix_function(pattern);
    using TransitionRow = std::array<std::size_t, 256>;
    std::vector<TransitionRow> transition(pattern.size());
    for (std::size_t state = 0; state < pattern.size(); ++state) {
        for (std::size_t byte = 0; byte < 256; ++byte) {
            const char current = static_cast<char>(static_cast<unsigned char>(byte));
            if (pattern[state] == current) {
                transition[state][byte] = state + 1;
            } else if (state == 0) {
                transition[state][byte] = 0;
            } else {
                transition[state][byte] = transition[prefix[state - 1]][byte];
            }
        }
    }

    std::size_t state = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto byte = static_cast<unsigned char>(text[i]);
        state = transition[state][byte];
        if (state == pattern.size()) {
            matches.push_back(i + 1 - pattern.size());
            state = prefix[state - 1];
        }
    }
    return matches;
}

} // namespace project47
