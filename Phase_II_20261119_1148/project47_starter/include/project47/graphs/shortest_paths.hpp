#pragma once

#include "project47/graphs/weighted_graph.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace project47 {

struct ShortestPathResult {
    std::string source;
    std::map<std::string, double> distance;
    std::map<std::string, std::string> predecessor;

    [[nodiscard]] std::vector<std::string> path_to(const std::string& target) const {
        const auto found = distance.find(target);
        if (found == distance.end()) throw std::invalid_argument("Target is not in the shortest-path result");
        if (found->second == std::numeric_limits<double>::infinity()) return {};

        std::vector<std::string> path;
        std::string current = target;
        path.push_back(current);
        while (current != source) {
            const auto previous = predecessor.find(current);
            if (previous == predecessor.end()) {
                throw std::logic_error("Shortest-path predecessor chain is incomplete");
            }
            current = previous->second;
            path.push_back(current);
            if (path.size() > distance.size()) throw std::logic_error("Cycle in predecessor chain");
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// Dijkstra's algorithm with a binary min-heap. Runtime: O((V + E) log V).
inline ShortestPathResult dijkstra(const WeightedGraph& graph, const std::string& source) {
    if (!graph.contains_vertex(source)) throw std::invalid_argument("Dijkstra source is not in the graph");

    const double infinity = std::numeric_limits<double>::infinity();
    ShortestPathResult result;
    result.source = source;
    for (const auto& [vertex, neighbors] : graph.adjacency()) {
        (void)neighbors;
        result.distance[vertex] = infinity;
    }
    result.distance[source] = 0.0;

    using QueueEntry = std::pair<double, std::string>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;
    queue.emplace(0.0, source);

    while (!queue.empty()) {
        const auto [current_distance, current] = queue.top();
        queue.pop();
        if (current_distance > result.distance.at(current)) continue;

        for (const auto& edge : graph.adjacency().at(current)) {
            const double candidate = current_distance + edge.weight;
            if (candidate < result.distance.at(edge.to)) {
                result.distance[edge.to] = candidate;
                result.predecessor[edge.to] = current;
                queue.emplace(candidate, edge.to);
            }
        }
    }
    return result;
}

} // namespace project47
