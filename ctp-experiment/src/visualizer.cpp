// Writes Graphviz DOT graphs with the blocked edge and initial route highlighted.
#include "visualizer.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>

#include <fstream>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace ctp {
namespace {

using NormalizedEdge = std::pair<Vertex, Vertex>;

NormalizedEdge normalize_edge(Vertex u, Vertex v) {
    return u < v ? NormalizedEdge{u, v} : NormalizedEdge{v, u};
}

struct VertexWriter {
    const GraphInstance& instance;

    void operator()(std::ostream& output, Vertex vertex) const {
        output << "[label=\"" << vertex << "\", style=filled, fillcolor=\"";
        if (vertex == instance.source) {
            output << "palegreen";
        } else if (vertex == instance.destination) {
            output << "lightblue";
        } else {
            output << "white";
        }
        output << "\"]";
    }
};

struct EdgeWriter {
    const GraphInstance& instance;
    const std::set<NormalizedEdge>& initialEdges;

    void operator()(
        std::ostream& output,
        boost::graph_traits<Graph>::edge_descriptor edge) const {
        const Vertex u = boost::source(edge, instance.graph);
        const Vertex v = boost::target(edge, instance.graph);
        const EdgeWeight weight =
            boost::get(boost::edge_weight, instance.graph, edge);
        output << "[label=\"" << weight << "\", ";
        if (normalize_edge(u, v) ==
            normalize_edge(instance.blockedEdge.u, instance.blockedEdge.v)) {
            output << "color=\"red\", style=dashed, penwidth=3";
        } else if (initialEdges.count(normalize_edge(u, v)) != 0) {
            output << "color=\"darkorange\", penwidth=2";
        } else {
            output << "color=\"gray\"";
        }
        output << "]";
    }
};

}  // namespace

void write_graph_dot(
    std::ostream& output,
    const GraphInstance& instance,
    const std::vector<int>& initialPath) {
    std::set<NormalizedEdge> initialEdges;
    for (std::size_t index = 1; index < initialPath.size(); ++index) {
        if (initialPath[index - 1] < 0 || initialPath[index] < 0) {
            throw std::invalid_argument("Initial path contains a negative vertex");
        }
        initialEdges.insert(normalize_edge(
            static_cast<Vertex>(initialPath[index - 1]),
            static_cast<Vertex>(initialPath[index])));
    }
    boost::write_graphviz(
        output,
        instance.graph,
        VertexWriter{instance},
        EdgeWriter{instance, initialEdges});
    if (!output) {
        throw std::runtime_error("Failed while writing Graphviz DOT output");
    }
}

void save_graph_dot(
    const GraphInstance& instance,
    const std::vector<int>& initialPath,
    const std::string& path) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot open DOT output file: " + path);
    }
    write_graph_dot(output, instance, initialPath);
}

}  // namespace ctp
