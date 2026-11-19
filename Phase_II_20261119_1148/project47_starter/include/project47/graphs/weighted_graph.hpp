#pragma once

#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace project47 {

struct WeightedNeighbor {
    std::string to;
    double weight{};
};

// Undirected weighted network connecting compute and storage nodes.
class WeightedGraph {
public:
    using Adjacency = std::map<std::string, std::vector<WeightedNeighbor>>;

    bool add_vertex(const std::string& id) {
        if (id.empty()) throw std::invalid_argument("Graph vertex ID cannot be empty");
        return adjacency_.try_emplace(id).second;
    }

    void add_edge(const std::string& from, const std::string& to, double weight) {
        if (from.empty() || to.empty()) throw std::invalid_argument("Graph vertex ID cannot be empty");
        if (from == to) throw std::invalid_argument("Self-loops are not allowed in the network graph");
        if (!std::isfinite(weight) || weight < 0.0) {
            throw std::invalid_argument("Network edge weight must be finite and non-negative");
        }
        auto from_it = adjacency_.find(from);
        auto to_it = adjacency_.find(to);
        if (from_it == adjacency_.end() || to_it == adjacency_.end()) {
            throw std::invalid_argument("Add both graph vertices before adding an edge");
        }
        from_it->second.push_back({to, weight});
        to_it->second.push_back({from, weight});
    }

    [[nodiscard]] bool contains_vertex(const std::string& id) const noexcept {
        return adjacency_.find(id) != adjacency_.end();
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept { return adjacency_.size(); }
    [[nodiscard]] const Adjacency& adjacency() const noexcept { return adjacency_; }

private:
    Adjacency adjacency_;
};

struct WeightedEdge {
    std::string from;
    std::string to;
    double weight{};
};

} // namespace project47
