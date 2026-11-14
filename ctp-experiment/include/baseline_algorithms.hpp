// Declares Dijkstra and the offline Backtrack baseline output and measurements.
#pragma once

#include "graph.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <vector>

namespace ctp {

struct DijkstraResult {
    bool reachable = false;
    double cost = 0.0;
    std::vector<int> path;
};

struct BacktrackScenario {
    std::size_t pathIndex = 0;
    EdgeEndpoints blockedEdge{0, 0};
    double initialPrefixCost = 0.0;
    double backtrackCost = 0.0;
    double rerouteCost = 0.0;
    double recoveryCost = 0.0;
    double totalTravelCost = 0.0;
    double offlineCost = 0.0;
    double competitiveRatio = 0.0;
};

struct BacktrackPlan {
    std::vector<std::vector<int>> paths;
    std::vector<int> initialPath;
    std::vector<BacktrackScenario> scenarios;
    double initialPathCost = 0.0;
    std::size_t estimatedStorageBytes = 0;
};

class InfeasibleRecoveryScenario final : public std::runtime_error {
public:
    InfeasibleRecoveryScenario(
        EdgeEndpoints blockedEdge,
        std::size_t pathIndex,
        std::vector<int> initialPath,
        double initialPathCost);

    EdgeEndpoints blocked_edge() const noexcept;
    std::size_t path_index() const noexcept;
    const std::vector<int>& initial_path() const noexcept;
    double initial_path_cost() const noexcept;

private:
    EdgeEndpoints blockedEdge_;
    std::size_t pathIndex_;
    std::vector<int> initialPath_;
    double initialPathCost_;
};

DijkstraResult run_dijkstra(
    const Graph& graph,
    Vertex source,
    Vertex destination,
    std::optional<EdgeEndpoints> excludedEdge = std::nullopt);

double path_cost(const Graph& graph, const std::vector<int>& path);
std::vector<std::vector<int>> run_backtrack(
    const Graph& graph,
    Vertex source,
    Vertex destination);
BacktrackPlan compute_backtrack_plan(
    const Graph& graph,
    Vertex source,
    Vertex destination);

}  // namespace ctp
