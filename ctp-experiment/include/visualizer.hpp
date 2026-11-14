// Declares Graphviz DOT output for a graph, blockage, and initial path.
#pragma once

#include "graph.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace ctp {

void write_graph_dot(
    std::ostream& output,
    const GraphInstance& instance,
    const std::vector<int>& initialPath);
void save_graph_dot(
    const GraphInstance& instance,
    const std::vector<int>& initialPath,
    const std::string& path);

}  // namespace ctp
