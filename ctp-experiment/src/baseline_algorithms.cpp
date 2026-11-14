// Implements BGL Dijkstra and offline Backtrack recovery for every path edge.
#include "baseline_algorithms.hpp"

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

#include <cmath>
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace ctp {
namespace {

bool same_edge(EdgeEndpoints a, EdgeEndpoints b) {
    return (a.u == b.u && a.v == b.v) || (a.u == b.v && a.v == b.u);
}

struct EdgeFilter {
    std::optional<EdgeEndpoints> excluded;
    const Graph* graph = nullptr;

    bool operator()(boost::graph_traits<Graph>::edge_descriptor edge) const {
        if (!excluded || graph == nullptr) {
            return true;
        }
        return !same_edge(
            {boost::source(edge, *graph), boost::target(edge, *graph)},
            *excluded);
    }
};

int output_vertex(Vertex vertex) {
    if (vertex > static_cast<Vertex>(std::numeric_limits<int>::max())) {
        throw std::overflow_error(
            "Vertex IDs exceed the algorithm output format's int range");
    }
    return static_cast<int>(vertex);
}

double edge_cost(const Graph& graph, int from, int to) {
    if (from < 0 || to < 0 ||
        static_cast<Vertex>(from) >= boost::num_vertices(graph) ||
        static_cast<Vertex>(to) >= boost::num_vertices(graph)) {
        throw std::invalid_argument("Path contains an out-of-range vertex");
    }
    const auto [edge, found] =
        boost::edge(static_cast<Vertex>(from), static_cast<Vertex>(to), graph);
    if (!found) {
        throw std::invalid_argument("Path contains a non-edge");
    }
    return static_cast<double>(
        boost::get(boost::edge_weight, graph, edge));
}

std::size_t estimate_plan_storage(const BacktrackPlan& plan) {
    std::size_t bytes =
        plan.paths.capacity() * sizeof(std::vector<int>) +
        plan.scenarios.capacity() * sizeof(BacktrackScenario) +
        plan.initialPath.capacity() * sizeof(int);
    for (const std::vector<int>& path : plan.paths) {
        bytes += path.capacity() * sizeof(int);
    }
    return bytes;
}

std::string infeasible_message(EdgeEndpoints edge, std::size_t pathIndex) {
    std::ostringstream message;
    message << "No recovery route exists when initial path edge ("
            << edge.u << ',' << edge.v << ") at index " << pathIndex
            << " is blocked";
    return message.str();
}

}  // namespace

InfeasibleRecoveryScenario::InfeasibleRecoveryScenario(
    EdgeEndpoints blockedEdge,
    std::size_t pathIndex,
    std::vector<int> initialPath,
    double initialPathCost)
    : std::runtime_error(infeasible_message(blockedEdge, pathIndex)),
      blockedEdge_(blockedEdge),
      pathIndex_(pathIndex),
      initialPath_(std::move(initialPath)),
      initialPathCost_(initialPathCost) {}

EdgeEndpoints InfeasibleRecoveryScenario::blocked_edge() const noexcept {
    return blockedEdge_;
}

std::size_t InfeasibleRecoveryScenario::path_index() const noexcept {
    return pathIndex_;
}

const std::vector<int>& InfeasibleRecoveryScenario::initial_path() const noexcept {
    return initialPath_;
}

double InfeasibleRecoveryScenario::initial_path_cost() const noexcept {
    return initialPathCost_;
}

DijkstraResult run_dijkstra(
    const Graph& graph,
    Vertex source,
    Vertex destination,
    std::optional<EdgeEndpoints> excludedEdge) {
    const std::size_t vertexCount = boost::num_vertices(graph);
    if (source >= vertexCount || destination >= vertexCount) {
        throw std::invalid_argument("Dijkstra source or destination is out of range");
    }
    if (vertexCount >
        static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1U) {
        throw std::overflow_error(
            "Graph vertex IDs exceed the algorithm output format's int range");
    }

    const double infinity = std::numeric_limits<double>::infinity();
    std::vector<double> distances(vertexCount, infinity);
    std::vector<Vertex> predecessors(vertexCount);
    auto distanceMap = boost::make_iterator_property_map(
        distances.begin(), boost::get(boost::vertex_index, graph));
    auto predecessorMap = boost::make_iterator_property_map(
        predecessors.begin(), boost::get(boost::vertex_index, graph));
    const EdgeFilter filter{excludedEdge, &graph};
    const boost::filtered_graph<const Graph, EdgeFilter> filtered(graph, filter);

    boost::dijkstra_shortest_paths(
        filtered,
        source,
        boost::weight_map(boost::get(boost::edge_weight, graph))
            .distance_map(distanceMap)
            .predecessor_map(predecessorMap)
            .distance_inf(infinity)
            .distance_zero(0.0));

    DijkstraResult result;
    if (!std::isfinite(distances[destination])) {
        return result;
    }

    std::vector<Vertex> reversedPath;
    Vertex current = destination;
    for (std::size_t steps = 0; steps < vertexCount; ++steps) {
        reversedPath.push_back(current);
        if (current == source) {
            break;
        }
        const Vertex predecessor = predecessors[current];
        if (predecessor == current || predecessor >= vertexCount) {
            throw std::logic_error("Dijkstra returned an invalid predecessor chain");
        }
        current = predecessor;
    }
    if (reversedPath.back() != source) {
        throw std::logic_error("Dijkstra predecessor chain contains a cycle");
    }

    result.reachable = true;
    result.cost = distances[destination];
    result.path.reserve(reversedPath.size());
    for (auto vertex = reversedPath.rbegin(); vertex != reversedPath.rend(); ++vertex) {
        result.path.push_back(output_vertex(*vertex));
    }
    return result;
}

double path_cost(const Graph& graph, const std::vector<int>& path) {
    if (path.empty()) {
        throw std::invalid_argument("Cannot measure the cost of an empty path");
    }
    double cost = 0.0;
    for (std::size_t index = 1; index < path.size(); ++index) {
        cost += edge_cost(graph, path[index - 1], path[index]);
    }
    return cost;
}

std::vector<std::vector<int>> run_backtrack(
    const Graph& graph,
    Vertex source,
    Vertex destination) {
    return compute_backtrack_plan(graph, source, destination).paths;
}

BacktrackPlan compute_backtrack_plan(
    const Graph& graph,
    Vertex source,
    Vertex destination) {
    const DijkstraResult initial =
        run_dijkstra(graph, source, destination);
    if (!initial.reachable) {
        throw std::runtime_error(
            "No initial source-to-destination route exists");
    }

    BacktrackPlan plan;
    plan.initialPath = initial.path;
    plan.initialPathCost = initial.cost;
    plan.paths.reserve(plan.initialPath.size());
    plan.scenarios.reserve(plan.initialPath.size() - 1);

    std::vector<double> prefixCosts(plan.initialPath.size(), 0.0);
    for (std::size_t index = 1; index < plan.initialPath.size(); ++index) {
        prefixCosts[index] =
            prefixCosts[index - 1] +
            edge_cost(graph, plan.initialPath[index - 1], plan.initialPath[index]);
    }

    for (std::size_t index = 0; index + 1 < plan.initialPath.size(); ++index) {
        const EdgeEndpoints blockedEdge{
            static_cast<Vertex>(plan.initialPath[index]),
            static_cast<Vertex>(plan.initialPath[index + 1])};
        const DijkstraResult reroute =
            run_dijkstra(graph, source, destination, blockedEdge);
        if (!reroute.reachable) {
            throw InfeasibleRecoveryScenario(
                blockedEdge, index, plan.initialPath, plan.initialPathCost);
        }

        std::vector<int> recovery;
        recovery.reserve(index + 1 + reroute.path.size() - 1);
        for (std::size_t prefix = index + 1; prefix > 0; --prefix) {
            recovery.push_back(plan.initialPath[prefix - 1]);
        }
        recovery.insert(recovery.end(), reroute.path.begin() + 1, reroute.path.end());
        plan.paths.push_back(std::move(recovery));

        BacktrackScenario scenario;
        scenario.pathIndex = index;
        scenario.blockedEdge = blockedEdge;
        scenario.initialPrefixCost = prefixCosts[index];
        scenario.backtrackCost = prefixCosts[index];
        scenario.rerouteCost = reroute.cost;
        scenario.recoveryCost =
            scenario.backtrackCost + scenario.rerouteCost;
        scenario.totalTravelCost =
            scenario.initialPrefixCost + scenario.recoveryCost;
        scenario.offlineCost = reroute.cost;
        scenario.competitiveRatio =
            scenario.totalTravelCost / scenario.offlineCost;
        plan.scenarios.push_back(scenario);
    }

    plan.paths.push_back({output_vertex(destination)});
    plan.estimatedStorageBytes = estimate_plan_storage(plan);
    return plan;
}

}  // namespace ctp
