#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace project47 {

struct LcsResult {
    std::size_t length{};
    std::string subsequence;
};

// Longest Common Subsequence. Runtime and memory: O(m * n).
// On ties during reconstruction, the algorithm moves upward in the DP table.
inline LcsResult longest_common_subsequence(const std::string& first,
                                            const std::string& second) {
    std::vector<std::vector<std::size_t>> dp(
        first.size() + 1, std::vector<std::size_t>(second.size() + 1, 0));

    for (std::size_t i = 1; i <= first.size(); ++i) {
        for (std::size_t j = 1; j <= second.size(); ++j) {
            if (first[i - 1] == second[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = dp[i - 1][j] > dp[i][j - 1]
                    ? dp[i - 1][j] : dp[i][j - 1];
            }
        }
    }

    LcsResult result;
    result.length = dp[first.size()][second.size()];
    std::size_t i = first.size();
    std::size_t j = second.size();
    while (i > 0 && j > 0) {
        if (first[i - 1] == second[j - 1]) {
            result.subsequence.push_back(first[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }
    for (std::size_t left = 0, right = result.subsequence.size(); left < right / 2; ++left) {
        const auto temp = result.subsequence[left];
        result.subsequence[left] = result.subsequence[right - left - 1];
        result.subsequence[right - left - 1] = temp;
    }
    return result;
}

} // namespace project47
