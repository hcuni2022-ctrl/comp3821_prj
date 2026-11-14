// Tests DOT output styles for source, destination, blocked edge, and initial path.
#include "visualizer.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_dot_annotations() {
    ctp::GraphInstance instance;
    instance.graph = ctp::Graph(3);
    instance.config = ctp::GraphConfig{3, 0.0, 1, 5, 1, std::nullopt};
    boost::add_edge(0, 1, 1, instance.graph);
    boost::add_edge(1, 2, 2, instance.graph);
    boost::add_edge(0, 2, 4, instance.graph);
    instance.source = 0;
    instance.destination = 2;
    instance.blockedEdge = {0, 2};

    std::ostringstream dot;
    ctp::write_graph_dot(dot, instance, {0, 1, 2});
    const std::string output = dot.str();
    require(output.find("fillcolor=\"palegreen\"") != std::string::npos,
            "DOT should mark the source");
    require(output.find("fillcolor=\"lightblue\"") != std::string::npos,
            "DOT should mark the destination");
    require(output.find("color=\"red\"") != std::string::npos,
            "DOT should highlight the blocked edge");
    require(output.find("color=\"darkorange\"") != std::string::npos,
            "DOT should highlight the initial path");
}

}  // namespace

int main() {
    try {
        test_dot_annotations();
    } catch (const std::exception& error) {
        std::cerr << "Visualizer test failure: " << error.what() << '\n';
        return 1;
    }
    std::cout << "Visualizer tests passed\n";
    return 0;
}
