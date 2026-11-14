// Tests Dijkstra and the Backtrack output contract on a known weighted graph.
#include "baseline_algorithms.hpp"

#include <boost/graph/graph_traits.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

ctp::Graph make_triangle() {
    ctp::Graph graph(3);
    boost::add_edge(0, 1, 1, graph);
    boost::add_edge(1, 2, 1, graph);
    boost::add_edge(0, 2, 3, graph);
    return graph;
}

void test_dijkstra_and_backtrack_output() {
    const ctp::Graph graph = make_triangle();
    const ctp::DijkstraResult shortest = ctp::run_dijkstra(graph, 0, 2);
    require(shortest.reachable, "Dijkstra should find the destination");
    require(shortest.path == std::vector<int>({0, 1, 2}),
            "Dijkstra should return the known shortest route");
    require(shortest.cost == 2.0, "Dijkstra should return the known route cost");

    const ctp::BacktrackPlan plan =
        ctp::compute_backtrack_plan(graph, 0, 2);
    require(plan.initialPath == std::vector<int>({0, 1, 2}),
            "The first elements should reconstruct the initial path");
    require(plan.paths ==
                std::vector<std::vector<int>>({{0, 2}, {1, 0, 2}, {2}}),
            "Backtrack output should return to source and end at t");
    require(plan.paths.back() == std::vector<int>({2}),
            "The output should end with the destination entry");
    require(ctp::run_backtrack(graph, 0, 2) == plan.paths,
            "The Backtrack entry point should return the required nested-vector output");
    require(plan.scenarios.size() == 2,
            "A recovery scenario should exist for every initial-path edge");
    require(plan.scenarios[0].totalTravelCost == 3.0 &&
                plan.scenarios[1].totalTravelCost == 5.0,
            "Backtrack totals should include prefix travel and return cost");
    require(plan.scenarios[1].recoveryCost == 4.0,
            "Recovery cost should include backtracking and the reroute");
    require(plan.estimatedStorageBytes > 0,
            "Plan storage should be measured");
    for (std::size_t index = 0; index < plan.scenarios.size(); ++index) {
        const auto& route = plan.paths[index];
        const auto& blocked = plan.scenarios[index].blockedEdge;
        require(route.front() == plan.initialPath[index] && route.back() == 2,
                "Each recovery route should start on the initial path and end at t");
        for (std::size_t step = 1; step < route.size(); ++step) {
            const int from = route[step - 1];
            const int to = route[step];
            require(!((from == static_cast<int>(blocked.u) &&
                       to == static_cast<int>(blocked.v)) ||
                      (from == static_cast<int>(blocked.v) &&
                       to == static_cast<int>(blocked.u))),
                    "A recovery route must avoid its blocked edge");
        }
    }
}

void test_infeasible_scenario_is_reported() {
    ctp::Graph graph(3);
    boost::add_edge(0, 1, 1, graph);
    boost::add_edge(1, 2, 1, graph);
    bool reported = false;
    try {
        (void)ctp::compute_backtrack_plan(graph, 0, 2);
    } catch (const ctp::InfeasibleRecoveryScenario& error) {
        reported = error.blocked_edge().u == 0 &&
                   error.blocked_edge().v == 1 &&
                   error.initial_path() == std::vector<int>({0, 1, 2});
    }
    require(reported,
            "A missing recovery route should report the blocked scenario explicitly");
}

}  // namespace

int main() {
    try {
        test_dijkstra_and_backtrack_output();
        test_infeasible_scenario_is_reported();
    } catch (const std::exception& error) {
        std::cerr << "Baseline test failure: " << error.what() << '\n';
        return 1;
    }
    std::cout << "Baseline tests passed\n";
    return 0;
}
