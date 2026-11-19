#pragma once

#include "project47/graphs/weighted_graph.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace project47 {

struct SpanningTreeResult {
    std::vector<WeightedEdge> edges;
    double total_weight{};
};

namespace detail {
struct CandidateEdge {
    double weight{};
    std::string from;
    std::string to;
};
struct CandidateEdgeGreater {
    bool operator()(const CandidateEdge& a, const CandidateEdge& b) const noexcept {
        if (a.weight != b.weight) return a.weight > b.weight;
        if (a.from != b.from) return a.from > b.from;
        return a.to > b.to;
    }
};

class DisjointSet {
public:
    explicit DisjointSet(const WeightedGraph::Adjacency& adjacency) {
        for (const auto& [vertex, neighbors] : adjacency) {
            (void)neighbors;
            parent_[vertex] = vertex;
            rank_[vertex] = 0;
        }
    }

    std::string find(const std::string& vertex) {
        auto& parent = parent_.at(vertex);
        if (parent != vertex) parent = find(parent);
        return parent;
    }

    bool unite(const std::string& a, const std::string& b) {
        std::string root_a = find(a);
        std::string root_b = find(b);
        if (root_a == root_b) return false;
        if (rank_.at(root_a) < rank_.at(root_b)) std::swap(root_a, root_b);
        parent_[root_b] = root_a;
        if (rank_.at(root_a) == rank_.at(root_b)) ++rank_[root_a];
        return true;
    }

private:
    std::map<std::string, std::string> parent_;
    std::map<std::string, std::size_t> rank_;
};
} // namespace detail

// Prim's greedy algorithm. A disconnected graph is rejected because it has no spanning tree.
// Runtime: O(E log E) with this adjacency-list / priority-queue implementation.
inline SpanningTreeResult prim_minimum_spanning_tree(const WeightedGraph& graph,
                                                       const std::string& start = {}) {
    if (graph.vertex_count() == 0) throw std::invalid_argument("Cannot build an MST of an empty graph");
    const std::string root = start.empty() ? graph.adjacency().begin()->first : start;
    if (!graph.contains_vertex(root)) throw std::invalid_argument("Prim start vertex is not in the graph");

    SpanningTreeResult result;
    std::map<std::string, bool> visited;
    std::priority_queue<detail::CandidateEdge, std::vector<detail::CandidateEdge>,
                        detail::CandidateEdgeGreater> candidates;

    auto add_frontier = [&](const std::string& vertex) {
        for (const auto& edge : graph.adjacency().at(vertex)) {
            if (!visited.at(edge.to)) candidates.push({edge.weight, vertex, edge.to});
        }
    };
    for (const auto& [vertex, neighbors] : graph.adjacency()) {
        (void)neighbors;
        visited[vertex] = false;
    }
    visited[root] = true;
    add_frontier(root);

    while (!candidates.empty() && result.edges.size() + 1 < graph.vertex_count()) {
        const auto edge = candidates.top();
        candidates.pop();
        if (visited.at(edge.to)) continue;
        visited[edge.to] = true;
        result.edges.push_back({edge.from, edge.to, edge.weight});
        result.total_weight += edge.weight;
        add_frontier(edge.to);
    }

    if (result.edges.size() + 1 != graph.vertex_count()) {
        throw std::logic_error("Network graph is disconnected; no spanning tree exists");
    }
    return result;
}

// Kruskal's algorithm with path-compressed, union-by-rank disjoint sets.
// Runtime: O(E log E).
inline SpanningTreeResult kruskal_minimum_spanning_tree(const WeightedGraph& graph) {
    if (graph.vertex_count() == 0) throw std::invalid_argument("Cannot build an MST of an empty graph");

    std::vector<WeightedEdge> edges;
    for (const auto& [from, neighbors] : graph.adjacency()) {
        for (const auto& edge : neighbors) {
            if (from < edge.to) edges.push_back({from, edge.to, edge.weight});
        }
    }
    std::sort(edges.begin(), edges.end(), [](const WeightedEdge& a, const WeightedEdge& b) {
        if (a.weight != b.weight) return a.weight < b.weight;
        if (a.from != b.from) return a.from < b.from;
        return a.to < b.to;
    });

    detail::DisjointSet sets(graph.adjacency());
    SpanningTreeResult result;
    for (const auto& edge : edges) {
        if (sets.unite(edge.from, edge.to)) {
            result.edges.push_back(edge);
            result.total_weight += edge.weight;
            if (result.edges.size() + 1 == graph.vertex_count()) break;
        }
    }
    if (result.edges.size() + 1 != graph.vertex_count()) {
        throw std::logic_error("Network graph is disconnected; no spanning tree exists");
    }
    return result;
}

} // namespace project47
