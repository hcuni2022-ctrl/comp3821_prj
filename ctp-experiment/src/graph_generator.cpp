// Implements seeded connected-graph generation with a guaranteed alternate route.
#include "graph_generator.hpp"

#include "graph_utils.hpp"

#include <boost/graph/graph_traits.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace ctp {
namespace {

void add_weighted_edge(
    Graph& graph,
    Vertex u,
    Vertex v,
    std::uniform_int_distribution<EdgeWeight>& weightDistribution,
    std::mt19937_64& random) {
    const auto [edge, inserted] =
        boost::add_edge(u, v, weightDistribution(random), graph);
    if (!inserted) {
        throw std::logic_error("Failed to add a generated graph edge");
    }
    (void)edge;
}

}  // namespace

GraphInstance generate_instance(const GraphConfig& config) {
    if (config.vertexCount < 3) {
        throw std::invalid_argument("At least three vertices are required");
    }
    if (!std::isfinite(config.edgeProbability) ||
        config.edgeProbability < 0.0 || config.edgeProbability > 1.0) {
        throw std::invalid_argument("Edge probability must be between 0 and 1");
    }
    if (config.minWeight <= 0 || config.maxWeight < config.minWeight) {
        throw std::invalid_argument(
            "Edge weights must be positive and min_weight <= max_weight");
    }

    GraphInstance instance;
    instance.graph = Graph(config.vertexCount);
    instance.config = config;

    std::mt19937_64 random(config.seed);
    const std::uint64_t blockageSeed =
        config.blockageSeed.value_or(config.seed);
    std::mt19937_64 scenarioRandom(blockageSeed);
    instance.config.blockageSeed = blockageSeed;
    std::uniform_int_distribution<EdgeWeight> weightDistribution(
        config.minWeight, config.maxWeight);
    std::bernoulli_distribution includeExtraEdge(config.edgeProbability);
    std::vector<Vertex> vertexOrder(config.vertexCount);
    for (Vertex vertex = 0; vertex < config.vertexCount; ++vertex) {
        vertexOrder[vertex] = vertex;
    }
    std::shuffle(vertexOrder.begin(), vertexOrder.end(), random);

    std::uniform_int_distribution<std::size_t> parentDistribution;
    for (std::size_t index = 1; index < vertexOrder.size(); ++index) {
        parentDistribution =
            std::uniform_int_distribution<std::size_t>(0, index - 1);
        add_weighted_edge(
            instance.graph,
            vertexOrder[index],
            vertexOrder[parentDistribution(random)],
            weightDistribution,
            random);
    }

    std::vector<EdgeEndpoints> possibleCycleEdges;
    for (Vertex u = 0; u < config.vertexCount; ++u) {
        for (Vertex v = u + 1; v < config.vertexCount; ++v) {
            if (!has_edge(instance.graph, u, v)) {
                possibleCycleEdges.push_back({u, v});
            }
        }
    }
    if (possibleCycleEdges.empty()) {
        throw std::logic_error("Generated spanning tree has no available cycle edge");
    }
    std::uniform_int_distribution<std::size_t> cycleEdgeDistribution(
        0, possibleCycleEdges.size() - 1);
    const EdgeEndpoints guaranteedCycleEdge =
        possibleCycleEdges[cycleEdgeDistribution(random)];
    add_weighted_edge(
        instance.graph,
        guaranteedCycleEdge.u,
        guaranteedCycleEdge.v,
        weightDistribution,
        random);

    for (const EdgeEndpoints& candidate : possibleCycleEdges) {
        if ((candidate.u != guaranteedCycleEdge.u ||
             candidate.v != guaranteedCycleEdge.v) &&
            includeExtraEdge(random)) {
            add_weighted_edge(
                instance.graph,
                candidate.u,
                candidate.v,
                weightDistribution,
                random);
        }
    }

    std::uniform_int_distribution<Vertex> vertexDistribution(
        0, config.vertexCount - 1);
    instance.source = vertexDistribution(scenarioRandom);
    do {
        instance.destination = vertexDistribution(scenarioRandom);
    } while (instance.destination == instance.source);

    std::vector<EdgeEndpoints> validBlockedEdges;
    const auto [edgeBegin, edgeEnd] = boost::edges(instance.graph);
    for (auto edge = edgeBegin; edge != edgeEnd; ++edge) {
        const Vertex u = boost::source(*edge, instance.graph);
        const Vertex v = boost::target(*edge, instance.graph);
        if (is_st_connected(
                instance.graph,
                instance.source,
                instance.destination,
                EdgeEndpoints{u, v})) {
            validBlockedEdges.push_back({u, v});
        }
    }

    if (validBlockedEdges.empty()) {
        throw std::logic_error(
            "Generated graph has no valid source-to-destination blockage");
    }

    std::uniform_int_distribution<std::size_t> edgeDistribution(
        0, validBlockedEdges.size() - 1);
    instance.blockedEdge = validBlockedEdges[edgeDistribution(scenarioRandom)];
    validate_graph_instance(instance);
    return instance;
}

}  // namespace ctp
