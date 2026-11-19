#pragma once

#include "project47/graphs/weighted_digraph.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct BellmanFordResult {
    std::string source;
    std::map<std::string, double> distance;
    std::map<std::string, std::string> predecessor;
    bool has_reachable_negative_cycle{};

    [[nodiscard]] std::vector<std::string> path_to(const std::string& target) const {
        if (has_reachable_negative_cycle) {
            throw std::logic_error("Shortest paths are undefined because a reachable negative cycle exists");
        }
        const auto found = distance.find(target);
        if (found == distance.end()) throw std::invalid_argument("Target is not in the Bellman-Ford result");
        if (found->second == std::numeric_limits<double>::infinity()) return {};
        std::vector<std::string> path{target};
        std::string current = target;
        while (current != source) {
            const auto previous = predecessor.find(current);
            if (previous == predecessor.end()) throw std::logic_error("Bellman-Ford predecessor chain is incomplete");
            current = previous->second;
            path.push_back(current);
            if (path.size() > distance.size()) throw std::logic_error("Cycle in Bellman-Ford predecessor chain");
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// Bellman-Ford supports negative edges and detects negative cycles reachable from source.
// Runtime: O(V * E), memory: O(V).
inline BellmanFordResult bellman_ford(const DirectedWeightedGraph& graph,
                                      const std::string& source) {
    if (!graph.contains_vertex(source)) throw std::invalid_argument("Bellman-Ford source is not in the graph");
    const double infinity = std::numeric_limits<double>::infinity();
    BellmanFordResult result;
    result.source = source;
    for (const auto& [vertex, neighbors] : graph.adjacency()) {
        (void)neighbors;
        result.distance.emplace(vertex, infinity);
    }
    result.distance[source] = 0.0;

    for (std::size_t pass = 1; pass < graph.vertex_count(); ++pass) {
        bool changed = false;
        for (const auto& edge : graph.edges()) {
            const double from_distance = result.distance.at(edge.from);
            if (from_distance == infinity) continue;
            const double candidate = from_distance + edge.weight;
            if (!std::isfinite(candidate)) throw std::overflow_error("Bellman-Ford distance overflow");
            if (candidate < result.distance.at(edge.to)) {
                result.distance[edge.to] = candidate;
                result.predecessor[edge.to] = edge.from;
                changed = true;
            }
        }
        if (!changed) break;
    }

    for (const auto& edge : graph.edges()) {
        const double from_distance = result.distance.at(edge.from);
        if (from_distance != infinity && from_distance + edge.weight < result.distance.at(edge.to)) {
            result.has_reachable_negative_cycle = true;
            break;
        }
    }
    return result;
}

} // namespace project47
