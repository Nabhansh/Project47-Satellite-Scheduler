#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace project47 {

struct FlowResult {
    std::uint64_t maximum_flow{};
    std::size_t augmentations{};
    std::vector<std::string> source_side_of_min_cut;
};

// Directed capacitated network. Ford-Fulkerson uses BFS augmenting paths
// (Edmonds-Karp), giving O(V * E^2) worst-case time.
class FlowNetwork {
public:
    using Capacity = std::uint64_t;

    bool add_vertex(const std::string& id) {
        if (id.empty()) throw std::invalid_argument("Flow-network vertex ID cannot be empty");
        const std::size_t new_index = ids_.size();
        const auto [it, inserted] = index_.emplace(id, new_index);
        (void)it;
        if (inserted) {
            ids_.push_back(id);
            adjacency_.emplace_back();
        }
        return inserted;
    }

    void add_edge(const std::string& from, const std::string& to, Capacity capacity) {
        if (from.empty() || to.empty()) throw std::invalid_argument("Flow edge endpoint cannot be empty");
        if (from == to) throw std::invalid_argument("Flow-network self-loops are not supported");
        const auto from_it = index_.find(from);
        const auto to_it = index_.find(to);
        if (from_it == index_.end() || to_it == index_.end()) {
            throw std::invalid_argument("Add both flow-network vertices before adding an edge");
        }

        const std::size_t u = from_it->second;
        const std::size_t v = to_it->second;
        const std::size_t reverse_for_forward = adjacency_[v].size();
        const std::size_t reverse_for_backward = adjacency_[u].size();
        adjacency_[u].push_back({v, reverse_for_forward, capacity});
        adjacency_[v].push_back({u, reverse_for_backward, 0});
    }

    [[nodiscard]] std::size_t vertex_count() const noexcept { return ids_.size(); }

    [[nodiscard]] FlowResult ford_fulkerson_max_flow(const std::string& source,
                                                      const std::string& sink) const {
        const auto source_it = index_.find(source);
        const auto sink_it = index_.find(sink);
        if (source_it == index_.end() || sink_it == index_.end()) {
            throw std::invalid_argument("Flow source and sink must exist in the network");
        }
        if (source_it->second == sink_it->second) {
            throw std::invalid_argument("Flow source and sink must be different");
        }

        auto residual = adjacency_;
        const std::size_t source_index = source_it->second;
        const std::size_t sink_index = sink_it->second;
        const std::size_t absent = ids_.size();
        FlowResult result;

        while (true) {
            std::vector<std::size_t> parent_vertex(ids_.size(), absent);
            std::vector<std::size_t> parent_edge(ids_.size(), absent);
            std::queue<std::size_t> frontier;
            parent_vertex[source_index] = source_index;
            frontier.push(source_index);

            while (!frontier.empty() && parent_vertex[sink_index] == absent) {
                const std::size_t current = frontier.front();
                frontier.pop();
                for (std::size_t edge_index = 0; edge_index < residual[current].size(); ++edge_index) {
                    const ResidualEdge& edge = residual[current][edge_index];
                    if (edge.capacity == 0 || parent_vertex[edge.to] != absent) continue;
                    parent_vertex[edge.to] = current;
                    parent_edge[edge.to] = edge_index;
                    frontier.push(edge.to);
                    if (edge.to == sink_index) break;
                }
            }

            if (parent_vertex[sink_index] == absent) break;

            Capacity bottleneck = std::numeric_limits<Capacity>::max();
            for (std::size_t vertex = sink_index; vertex != source_index;
                 vertex = parent_vertex[vertex]) {
                const ResidualEdge& edge = residual[parent_vertex[vertex]][parent_edge[vertex]];
                if (edge.capacity < bottleneck) bottleneck = edge.capacity;
            }
            if (bottleneck == 0) throw std::logic_error("Residual path has zero bottleneck");
            if (result.maximum_flow > std::numeric_limits<Capacity>::max() - bottleneck) {
                throw std::overflow_error("Maximum flow exceeds uint64_t capacity");
            }

            for (std::size_t vertex = sink_index; vertex != source_index;
                 vertex = parent_vertex[vertex]) {
                const std::size_t previous = parent_vertex[vertex];
                const std::size_t edge_index = parent_edge[vertex];
                ResidualEdge& edge = residual[previous][edge_index];
                ResidualEdge& reverse = residual[vertex][edge.reverse];
                edge.capacity -= bottleneck;
                if (reverse.capacity > std::numeric_limits<Capacity>::max() - bottleneck) {
                    throw std::overflow_error("Residual capacity exceeds uint64_t range");
                }
                reverse.capacity += bottleneck;
            }
            result.maximum_flow += bottleneck;
            ++result.augmentations;
        }

        std::vector<bool> reachable(ids_.size(), false);
        std::queue<std::size_t> frontier;
        reachable[source_index] = true;
        frontier.push(source_index);
        while (!frontier.empty()) {
            const std::size_t current = frontier.front();
            frontier.pop();
            for (const ResidualEdge& edge : residual[current]) {
                if (edge.capacity > 0 && !reachable[edge.to]) {
                    reachable[edge.to] = true;
                    frontier.push(edge.to);
                }
            }
        }
        for (std::size_t i = 0; i < ids_.size(); ++i) {
            if (reachable[i]) result.source_side_of_min_cut.push_back(ids_[i]);
        }
        return result;
    }

private:
    struct ResidualEdge {
        std::size_t to{};
        std::size_t reverse{};
        Capacity capacity{};
    };

    std::vector<std::string> ids_;
    std::map<std::string, std::size_t> index_;
    std::vector<std::vector<ResidualEdge>> adjacency_;
};

} // namespace project47
