#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <vector>

namespace project47 {

// Directed acyclic graph of job dependencies. An edge A -> B means job A
// must complete before job B can start. Topological order is deterministic:
// among currently-ready jobs, the smallest ID is selected first.
class DependencyGraph {
public:
    using JobId = std::uint64_t;

    void add_job(JobId id) {
        if (id == 0) {
            throw std::invalid_argument("Dependency graph job ID must be non-zero");
        }
        adjacency_.try_emplace(id);
    }

    // Returns true when a new edge is inserted; duplicate edges return false.
    bool add_dependency(JobId prerequisite, JobId dependent) {
        if (prerequisite == 0 || dependent == 0) {
            throw std::invalid_argument("Dependency graph job ID must be non-zero");
        }
        if (adjacency_.find(prerequisite) == adjacency_.end() ||
            adjacency_.find(dependent) == adjacency_.end()) {
            throw std::invalid_argument("Both jobs must be added before adding a dependency");
        }
        if (prerequisite == dependent) {
            throw std::invalid_argument("A job cannot depend on itself");
        }
        return adjacency_.at(prerequisite).insert(dependent).second;
    }

    [[nodiscard]] std::size_t job_count() const noexcept {
        return adjacency_.size();
    }

    [[nodiscard]] std::size_t dependency_count() const noexcept {
        std::size_t count = 0;
        for (const auto& [job, dependents] : adjacency_) {
            (void)job;
            count += dependents.size();
        }
        return count;
    }

    [[nodiscard]] std::vector<JobId> topological_order() const {
        std::map<JobId, std::size_t> indegree;
        for (const auto& [job, dependents] : adjacency_) {
            indegree.try_emplace(job, 0);
            for (const JobId dependent : dependents) {
                ++indegree[dependent];
            }
        }

        std::priority_queue<JobId, std::vector<JobId>, std::greater<JobId>> ready;
        for (const auto& [job, degree] : indegree) {
            if (degree == 0) {
                ready.push(job);
            }
        }

        std::vector<JobId> order;
        order.reserve(adjacency_.size());
        while (!ready.empty()) {
            const JobId current = ready.top();
            ready.pop();
            order.push_back(current);

            for (const JobId dependent : adjacency_.at(current)) {
                auto& degree = indegree.at(dependent);
                --degree;
                if (degree == 0) {
                    ready.push(dependent);
                }
            }
        }

        if (order.size() != adjacency_.size()) {
            throw std::logic_error("Job dependency graph contains a cycle");
        }
        return order;
    }

    [[nodiscard]] bool has_cycle() const {
        try {
            (void)topological_order();
            return false;
        } catch (const std::logic_error&) {
            return true;
        }
    }

private:
    std::map<JobId, std::set<JobId>> adjacency_;
};

} // namespace project47
