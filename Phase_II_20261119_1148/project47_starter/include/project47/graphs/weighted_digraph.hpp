#pragma once

#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct DirectedWeightedEdge {
    std::string from;
    std::string to;
    double weight{};
};

// Directed graph used by algorithms that support negative edge weights.
// Unlike WeightedGraph, this type permits negative edges and self-loops.
class DirectedWeightedGraph {
public:
    bool add_vertex(const std::string& id) {
        if (id.empty()) throw std::invalid_argument("Graph vertex ID cannot be empty");
        return adjacency_.try_emplace(id).second;
    }

    void add_edge(const std::string& from, const std::string& to, double weight) {
        if (from.empty() || to.empty()) throw std::invalid_argument("Graph vertex ID cannot be empty");
        if (!std::isfinite(weight)) throw std::invalid_argument("Directed edge weight must be finite");
        const auto from_it = adjacency_.find(from);
        const auto to_it = adjacency_.find(to);
        if (from_it == adjacency_.end() || to_it == adjacency_.end()) {
            throw std::invalid_argument("Add both graph vertices before adding an edge");
        }
        edges_.push_back({from, to, weight});
        from_it->second.push_back({to, weight});
    }

    [[nodiscard]] bool contains_vertex(const std::string& id) const noexcept {
        return adjacency_.find(id) != adjacency_.end();
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept { return adjacency_.size(); }
    [[nodiscard]] const std::map<std::string, std::vector<std::pair<std::string, double>>>&
    adjacency() const noexcept { return adjacency_; }
    [[nodiscard]] const std::vector<DirectedWeightedEdge>& edges() const noexcept { return edges_; }

private:
    std::map<std::string, std::vector<std::pair<std::string, double>>> adjacency_;
    std::vector<DirectedWeightedEdge> edges_;
};

} // namespace project47
