#pragma once

#include "project47/graphs/weighted_digraph.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace project47 {

struct FloydWarshallResult {
    std::vector<std::string> vertices;
    std::vector<std::vector<double>> distance;
    std::vector<std::vector<std::optional<std::size_t>>> next;
    bool has_negative_cycle{};

    [[nodiscard]] double distance_between(const std::string& from, const std::string& to) const {
        const auto source_it = index.find(from);
        const auto target_it = index.find(to);
        if (source_it == index.end() || target_it == index.end()) {
            throw std::invalid_argument("Floyd-Warshall vertex is not in the result");
        }
        return distance[source_it->second][target_it->second];
    }

    [[nodiscard]] std::vector<std::string> path(const std::string& from,
                                                const std::string& to) const {
        if (has_negative_cycle) {
            throw std::logic_error("All-pairs shortest paths are undefined because a negative cycle exists");
        }
        const auto source_it = index.find(from);
        const auto target_it = index.find(to);
        if (source_it == index.end() || target_it == index.end()) {
            throw std::invalid_argument("Floyd-Warshall vertex is not in the result");
        }
        const std::size_t source = source_it->second;
        const std::size_t target = target_it->second;
        if (!next[source][target]) return {};

        std::vector<std::string> result{vertices[source]};
        std::size_t current = source;
        while (current != target) {
            const auto following = next[current][target];
            if (!following) return {};
            current = *following;
            result.push_back(vertices[current]);
            if (result.size() > vertices.size() + 1) {
                throw std::logic_error("Cycle in Floyd-Warshall path reconstruction");
            }
        }
        return result;
    }

private:
    std::unordered_map<std::string, std::size_t> index;
    friend FloydWarshallResult floyd_warshall(const DirectedWeightedGraph& graph);
};

// Floyd-Warshall computes all-pairs shortest paths, including graphs with negative edges.
// Runtime: O(V^3), memory: O(V^2). Negative cycles are reported explicitly.
inline FloydWarshallResult floyd_warshall(const DirectedWeightedGraph& graph) {
    FloydWarshallResult result;
    constexpr double infinity = std::numeric_limits<double>::infinity();
    for (const auto& [vertex, neighbors] : graph.adjacency()) {
        (void)neighbors;
        result.index.emplace(vertex, result.vertices.size());
        result.vertices.push_back(vertex);
    }

    const std::size_t n = result.vertices.size();
    result.distance.assign(n, std::vector<double>(n, infinity));
    result.next.assign(n, std::vector<std::optional<std::size_t>>(n));
    for (std::size_t i = 0; i < n; ++i) {
        result.distance[i][i] = 0.0;
        result.next[i][i] = i;
    }
    for (const auto& edge : graph.edges()) {
        const std::size_t from = result.index.at(edge.from);
        const std::size_t to = result.index.at(edge.to);
        if (edge.weight < result.distance[from][to]) {
            result.distance[from][to] = edge.weight;
            result.next[from][to] = to;
        }
    }

    for (std::size_t k = 0; k < n; ++k) {
        for (std::size_t i = 0; i < n; ++i) {
            if (result.distance[i][k] == infinity) continue;
            for (std::size_t j = 0; j < n; ++j) {
                if (result.distance[k][j] == infinity) continue;
                const double candidate = result.distance[i][k] + result.distance[k][j];
                if (!std::isfinite(candidate)) throw std::overflow_error("Floyd-Warshall distance overflow");
                if (candidate < result.distance[i][j]) {
                    result.distance[i][j] = candidate;
                    result.next[i][j] = result.next[i][k];
                }
            }
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        if (result.distance[i][i] < 0.0) {
            result.has_negative_cycle = true;
            break;
        }
    }
    return result;
}

} // namespace project47
