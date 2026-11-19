#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct MatchingPair {
    std::string left;
    std::string right;
};

struct BipartiteMatchingResult {
    std::vector<MatchingPair> pairs;
    [[nodiscard]] std::size_t cardinality() const noexcept { return pairs.size(); }
};

// Maximum-cardinality two-sided assignment using augmenting paths (Kuhn's algorithm).
// Typical implementation complexity is O(V * E).
class BipartiteGraph {
public:
    bool add_left_vertex(const std::string& id) {
        validate_id(id);
        return left_.insert(id).second;
    }

    bool add_right_vertex(const std::string& id) {
        validate_id(id);
        return right_.insert(id).second;
    }

    bool add_edge(const std::string& left_id, const std::string& right_id) {
        if (!left_.contains(left_id) || !right_.contains(right_id)) {
            throw std::invalid_argument("Bipartite edge endpoints must be declared on their respective sides");
        }
        auto& neighbors = adjacency_[left_id];
        if (std::find(neighbors.begin(), neighbors.end(), right_id) != neighbors.end()) return false;
        neighbors.push_back(right_id);
        return true;
    }

    [[nodiscard]] std::size_t left_count() const noexcept { return left_.size(); }
    [[nodiscard]] std::size_t right_count() const noexcept { return right_.size(); }

    [[nodiscard]] BipartiteMatchingResult maximum_matching() const {
        std::map<std::string, std::string> matched_right_to_left;
        std::function<bool(const std::string&, std::set<std::string>&)> augment;
        augment = [&](const std::string& left_id, std::set<std::string>& visited_right) {
            const auto neighbors_it = adjacency_.find(left_id);
            if (neighbors_it == adjacency_.end()) return false;
            for (const std::string& right_id : neighbors_it->second) {
                if (!visited_right.insert(right_id).second) continue;
                const auto matched_it = matched_right_to_left.find(right_id);
                if (matched_it == matched_right_to_left.end() ||
                    augment(matched_it->second, visited_right)) {
                    matched_right_to_left[right_id] = left_id;
                    return true;
                }
            }
            return false;
        };

        for (const std::string& left_id : left_) {
            std::set<std::string> visited_right;
            (void)augment(left_id, visited_right);
        }

        BipartiteMatchingResult result;
        result.pairs.reserve(matched_right_to_left.size());
        for (const auto& [right_id, left_id] : matched_right_to_left) {
            result.pairs.push_back({left_id, right_id});
        }
        std::sort(result.pairs.begin(), result.pairs.end(),
                  [](const MatchingPair& a, const MatchingPair& b) { return a.left < b.left; });
        return result;
    }

private:
    static void validate_id(const std::string& id) {
        if (id.empty()) throw std::invalid_argument("Bipartite vertex ID cannot be empty");
    }

    std::set<std::string> left_;
    std::set<std::string> right_;
    std::map<std::string, std::vector<std::string>> adjacency_;
};

} // namespace project47
