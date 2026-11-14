// Declares graph connectivity, instance validation, and graph persistence helpers.
#pragma once

#include "graph.hpp"

#include <optional>
#include <string>

namespace ctp {

bool has_edge(const Graph& graph, Vertex u, Vertex v);

bool is_st_connected(
    const Graph& graph,
    Vertex source,
    Vertex destination,
    std::optional<EdgeEndpoints> excludedEdge = std::nullopt);

void validate_graph_instance(const GraphInstance& instance);
void save_graph_instance(const GraphInstance& instance, const std::string& path);
GraphInstance load_graph_instance(const std::string& path);

}  // namespace ctp
