// Implements BGL-backed connectivity checks and graph-instance serialization.
#include "graph_utils.hpp"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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

template <typename Value>
Value parse_value(const std::string& text, const std::string& field) {
    if constexpr (std::is_unsigned_v<Value>) {
        if (!text.empty() && text.front() == '-') {
            throw std::runtime_error("Invalid negative value for " + field);
        }
    }

    std::istringstream input(text);
    Value value{};
    if (!(input >> value)) {
        throw std::runtime_error("Invalid value for " + field);
    }
    input >> std::ws;
    if (!input.eof()) {
        throw std::runtime_error("Unexpected trailing data for " + field);
    }
    return value;
}

const std::string& required_metadata(
    const std::map<std::string, std::string>& metadata,
    const std::string& key) {
    const auto value = metadata.find(key);
    if (value == metadata.end()) {
        throw std::runtime_error("Missing graph metadata: " + key);
    }
    return value->second;
}

struct SavedEdge {
    Vertex u;
    Vertex v;
    EdgeWeight weight;
};

}  // namespace

bool has_edge(const Graph& graph, Vertex u, Vertex v) {
    if (u >= boost::num_vertices(graph) || v >= boost::num_vertices(graph)) {
        return false;
    }
    return boost::edge(u, v, graph).second;
}

bool is_st_connected(
    const Graph& graph,
    Vertex source,
    Vertex destination,
    std::optional<EdgeEndpoints> excludedEdge) {
    const std::size_t vertexCount = boost::num_vertices(graph);
    if (source >= vertexCount || destination >= vertexCount) {
        return false;
    }
    if (source == destination) {
        return true;
    }

    using FilteredGraph = boost::filtered_graph<const Graph, EdgeFilter>;
    const EdgeFilter filter{excludedEdge, &graph};
    const FilteredGraph filtered(graph, filter);
    std::vector<boost::default_color_type> colors(vertexCount);
    auto colorMap = boost::make_iterator_property_map(
        colors.begin(), boost::get(boost::vertex_index, graph));
    boost::breadth_first_search(
        filtered, source, boost::color_map(colorMap));
    return boost::get(colorMap, destination) != boost::white_color;
}

void validate_graph_instance(const GraphInstance& instance) {
    const std::size_t vertexCount = boost::num_vertices(instance.graph);
    if (vertexCount < 3 || instance.config.vertexCount != vertexCount) {
        throw std::invalid_argument(
            "Graph vertex count does not match valid configuration");
    }
    if (!std::isfinite(instance.config.edgeProbability) ||
        instance.config.edgeProbability < 0.0 ||
        instance.config.edgeProbability > 1.0) {
        throw std::invalid_argument("Invalid edge probability");
    }
    if (instance.config.minWeight <= 0 ||
        instance.config.maxWeight < instance.config.minWeight) {
        throw std::invalid_argument("Invalid edge-weight range");
    }
    if (instance.source >= vertexCount ||
        instance.destination >= vertexCount ||
        instance.source == instance.destination) {
        throw std::invalid_argument("Invalid source or destination vertex");
    }

    const auto weightMap = boost::get(boost::edge_weight, instance.graph);
    const auto [edgeBegin, edgeEnd] = boost::edges(instance.graph);
    for (auto edge = edgeBegin; edge != edgeEnd; ++edge) {
        const EdgeWeight weight = boost::get(weightMap, *edge);
        if (weight <= 0 || weight < instance.config.minWeight ||
            weight > instance.config.maxWeight) {
            throw std::invalid_argument(
                "Graph contains an edge weight outside its valid range");
        }
    }

    if (instance.blockedEdge.u >= vertexCount ||
        instance.blockedEdge.v >= vertexCount ||
        instance.blockedEdge.u == instance.blockedEdge.v ||
        !has_edge(
            instance.graph, instance.blockedEdge.u, instance.blockedEdge.v)) {
        throw std::invalid_argument("Blocked edge is not present in the graph");
    }
    if (!is_st_connected(
            instance.graph,
            instance.source,
            instance.destination,
            instance.blockedEdge)) {
        throw std::invalid_argument(
            "No source-to-destination route remains after blockage");
    }
}

void save_graph_instance(const GraphInstance& instance, const std::string& path) {
    validate_graph_instance(instance);
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot open graph output file: " + path);
    }

    output << "# ctp-experiment-instance-v1\n"
           << "# vertex_count=" << instance.config.vertexCount << '\n'
           << "# edge_probability="
           << std::setprecision(std::numeric_limits<double>::max_digits10)
           << instance.config.edgeProbability << '\n'
           << "# min_weight=" << instance.config.minWeight << '\n'
           << "# max_weight=" << instance.config.maxWeight << '\n'
           << "# seed=" << instance.config.seed << '\n'
           << "# blockage_seed="
           << instance.config.blockageSeed.value_or(instance.config.seed) << '\n'
           << "# source=" << instance.source << '\n'
           << "# destination=" << instance.destination << '\n'
           << "# blocked_edge=" << instance.blockedEdge.u << ','
           << instance.blockedEdge.v << '\n'
           << "# edges: u v weight\n";

    std::vector<SavedEdge> edges;
    const auto weightMap = boost::get(boost::edge_weight, instance.graph);
    const auto [edgeBegin, edgeEnd] = boost::edges(instance.graph);
    for (auto edge = edgeBegin; edge != edgeEnd; ++edge) {
        Vertex u = boost::source(*edge, instance.graph);
        Vertex v = boost::target(*edge, instance.graph);
        if (v < u) {
            std::swap(u, v);
        }
        edges.push_back({u, v, boost::get(weightMap, *edge)});
    }
    std::sort(edges.begin(), edges.end(), [](const SavedEdge& a, const SavedEdge& b) {
        return a.u < b.u || (a.u == b.u && a.v < b.v);
    });
    for (const SavedEdge& edge : edges) {
        output << edge.u << ' ' << edge.v << ' ' << edge.weight << '\n';
    }
    if (!output) {
        throw std::runtime_error("Failed while writing graph output file: " + path);
    }
}

GraphInstance load_graph_instance(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open graph input file: " + path);
    }

    std::string line;
    if (!std::getline(input, line) || line != "# ctp-experiment-instance-v1") {
        throw std::runtime_error("Unsupported or missing graph file header");
    }

    std::map<std::string, std::string> metadata;
    std::vector<SavedEdge> edges;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        if (line.front() == '#') {
            const std::size_t separator = line.find('=');
            if (separator != std::string::npos) {
                const std::string key = line.substr(2, separator - 2);
                const std::string value = line.substr(separator + 1);
                if (!metadata.emplace(key, value).second) {
                    throw std::runtime_error("Duplicate graph metadata: " + key);
                }
            }
            continue;
        }

        std::istringstream row(line);
        SavedEdge edge{};
        std::string extra;
        if (!(row >> edge.u >> edge.v >> edge.weight) || (row >> extra)) {
            throw std::runtime_error("Malformed graph edge row: " + line);
        }
        edges.push_back(edge);
    }
    if (!input.eof()) {
        throw std::runtime_error("Failed while reading graph input file: " + path);
    }

    GraphInstance instance;
    instance.config.vertexCount =
        parse_value<std::size_t>(required_metadata(metadata, "vertex_count"), "vertex_count");
    instance.config.edgeProbability =
        parse_value<double>(required_metadata(metadata, "edge_probability"), "edge_probability");
    instance.config.minWeight =
        parse_value<EdgeWeight>(required_metadata(metadata, "min_weight"), "min_weight");
    instance.config.maxWeight =
        parse_value<EdgeWeight>(required_metadata(metadata, "max_weight"), "max_weight");
    instance.config.seed =
        parse_value<std::uint64_t>(required_metadata(metadata, "seed"), "seed");
    const auto blockageSeed = metadata.find("blockage_seed");
    instance.config.blockageSeed =
        blockageSeed == metadata.end()
            ? instance.config.seed
            : parse_value<std::uint64_t>(blockageSeed->second, "blockage_seed");
    instance.source =
        parse_value<Vertex>(required_metadata(metadata, "source"), "source");
    instance.destination =
        parse_value<Vertex>(required_metadata(metadata, "destination"), "destination");

    const std::string blockedText =
        required_metadata(metadata, "blocked_edge");
    const std::size_t comma = blockedText.find(',');
    if (comma == std::string::npos ||
        blockedText.find(',', comma + 1) != std::string::npos) {
        throw std::runtime_error("Malformed blocked_edge metadata");
    }
    instance.blockedEdge = {
        parse_value<Vertex>(blockedText.substr(0, comma), "blocked_edge"),
        parse_value<Vertex>(blockedText.substr(comma + 1), "blocked_edge")};

    instance.graph = Graph(instance.config.vertexCount);
    for (const SavedEdge& edge : edges) {
        if (edge.u >= instance.config.vertexCount ||
            edge.v >= instance.config.vertexCount || edge.u == edge.v) {
            throw std::runtime_error("Graph edge endpoint is out of range");
        }
        if (edge.weight <= 0 || edge.weight < instance.config.minWeight ||
            edge.weight > instance.config.maxWeight) {
            throw std::runtime_error("Graph edge weight is outside configured bounds");
        }
        const auto [descriptor, inserted] =
            boost::add_edge(edge.u, edge.v, edge.weight, instance.graph);
        (void)descriptor;
        if (!inserted) {
            throw std::runtime_error("Graph file contains a duplicate edge");
        }
    }

    validate_graph_instance(instance);
    return instance;
}

}  // namespace ctp
