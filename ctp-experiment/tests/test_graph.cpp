// Tests graph generation, blockage validation, and saved-instance round trips.
#include "graph_generator.hpp"
#include "graph_utils.hpp"

#include <boost/graph/graph_traits.hpp>

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

using EdgeRecord = std::tuple<ctp::Vertex, ctp::Vertex, ctp::EdgeWeight>;

std::vector<EdgeRecord> edges_of(const ctp::Graph& graph) {
    std::vector<EdgeRecord> records;
    const auto weights = boost::get(boost::edge_weight, graph);
    const auto [begin, end] = boost::edges(graph);
    for (auto edge = begin; edge != end; ++edge) {
        ctp::Vertex u = boost::source(*edge, graph);
        ctp::Vertex v = boost::target(*edge, graph);
        if (v < u) {
            std::swap(u, v);
        }
        records.emplace_back(u, v, boost::get(weights, *edge));
    }
    std::sort(records.begin(), records.end());
    return records;
}

void test_generation_and_reproducibility() {
    const ctp::GraphConfig config{12, 0.35, 2, 17, 19427, std::nullopt};
    const ctp::GraphInstance first = ctp::generate_instance(config);
    const ctp::GraphInstance second = ctp::generate_instance(config);

    require(boost::num_vertices(first.graph) == config.vertexCount,
            "Generated graph has the wrong number of vertices");
    require(edges_of(first.graph) == edges_of(second.graph),
            "Equal seeds should generate identical weighted graphs");
    require(first.source == second.source &&
                first.destination == second.destination &&
                first.blockedEdge.u == second.blockedEdge.u &&
                first.blockedEdge.v == second.blockedEdge.v,
            "Equal seeds should generate identical instance metadata");
    require(ctp::is_st_connected(first.graph, first.source, first.destination),
            "Generated graph should connect source and destination");
    require(ctp::is_st_connected(
                first.graph,
                first.source,
                first.destination,
                first.blockedEdge),
            "Generated blockage should preserve an s-to-t route");

    const auto weights = boost::get(boost::edge_weight, first.graph);
    const auto [begin, end] = boost::edges(first.graph);
    for (auto edge = begin; edge != end; ++edge) {
        const ctp::EdgeWeight weight = boost::get(weights, *edge);
        require(weight >= config.minWeight && weight <= config.maxWeight,
                "Generated edge weight is outside configured bounds");
    }
}

void test_save_and_load_round_trip() {
    const ctp::GraphConfig config{8, 0.5, 1, 25, 73, std::nullopt};
    const ctp::GraphInstance generated = ctp::generate_instance(config);
    const std::filesystem::path path = "build/test-instance.ctp";
    ctp::save_graph_instance(generated, path.string());
    const ctp::GraphInstance loaded = ctp::load_graph_instance(path.string());

    require(edges_of(generated.graph) == edges_of(loaded.graph),
            "Loaded graph edges differ from the saved graph");
    require(generated.config.vertexCount == loaded.config.vertexCount &&
                generated.config.edgeProbability ==
                    loaded.config.edgeProbability &&
                generated.config.minWeight == loaded.config.minWeight &&
                generated.config.maxWeight == loaded.config.maxWeight &&
                generated.config.seed == loaded.config.seed &&
                generated.config.blockageSeed == loaded.config.blockageSeed,
            "Loaded graph configuration differs from the saved configuration");
    require(generated.source == loaded.source &&
                generated.destination == loaded.destination &&
                generated.blockedEdge.u == loaded.blockedEdge.u &&
                generated.blockedEdge.v == loaded.blockedEdge.v,
            "Loaded graph endpoints differ from the saved instance");
    require(ctp::is_st_connected(
                loaded.graph,
                loaded.source,
                loaded.destination,
                loaded.blockedEdge),
            "Loaded blockage should preserve an s-to-t route");

    std::filesystem::remove(path);
}

void test_invalid_configuration() {
    bool rejected = false;
    try {
        (void)ctp::generate_instance(
            ctp::GraphConfig{2, 0.5, 1, 5, 1, std::nullopt});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "Generator should reject graphs with fewer than 3 vertices");

    rejected = false;
    try {
        (void)ctp::generate_instance(
            ctp::GraphConfig{4, 1.1, 1, 5, 1, std::nullopt});
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected, "Generator should reject invalid edge probabilities");
}

void test_blockage_connectivity_validation() {
    ctp::GraphInstance instance;
    instance.graph = ctp::Graph(4);
    instance.config = ctp::GraphConfig{4, 0.0, 1, 4, 9, std::nullopt};
    boost::add_edge(0, 1, 1, instance.graph);
    boost::add_edge(1, 2, 2, instance.graph);
    boost::add_edge(2, 0, 3, instance.graph);
    boost::add_edge(2, 3, 4, instance.graph);
    instance.source = 0;
    instance.destination = 3;
    instance.blockedEdge = {2, 3};

    bool rejected = false;
    try {
        ctp::validate_graph_instance(instance);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    require(rejected,
            "Validation should reject a blockage that disconnects source and destination");
}

}  // namespace

int main() {
    try {
        test_generation_and_reproducibility();
        test_save_and_load_round_trip();
        test_invalid_configuration();
        test_blockage_connectivity_validation();
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
    std::cout << "Graph infrastructure tests passed\n";
    return 0;
}
