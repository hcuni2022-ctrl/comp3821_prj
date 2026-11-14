// Defines the BGL graph model and reproducible experiment-instance data.
#pragma once

#include <boost/graph/adjacency_list.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ctp {

using Vertex = std::size_t;
using EdgeWeight = int;
using Graph = boost::adjacency_list<
    boost::setS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    boost::property<boost::edge_weight_t, EdgeWeight>>;

struct EdgeEndpoints {
    Vertex u;
    Vertex v;
};

struct GraphConfig {
    std::size_t vertexCount = 0;
    double edgeProbability = 0.0;
    EdgeWeight minWeight = 1;
    EdgeWeight maxWeight = 1;
    std::uint64_t seed = 0;
    std::optional<std::uint64_t> blockageSeed;
};

struct GraphInstance {
    Graph graph;
    Vertex source = 0;
    Vertex destination = 0;
    EdgeEndpoints blockedEdge{0, 0};
    GraphConfig config;
};

}  // namespace ctp
