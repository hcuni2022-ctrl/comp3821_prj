// Tests config loading, experiment execution, CSV logging, and saved artifacts.
#include "config.hpp"
#include "experiment.hpp"
#include "graph_utils.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string read_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Unable to read test output file");
    }
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

void test_configured_experiment_batch() {
    const std::string configPath = "build/test-config.csv";
    {
        std::ofstream config(configPath);
        config << "# experiment test\n"
               << "vertex_count,edge_probability,min_weight,max_weight,"
                  "edge_seed,blockage_seed,repetitions\n"
               << "3,1.0,1,5,41,73,1\n";
    }

    const std::vector<ctp::ExperimentConfig> configurations =
        ctp::load_experiment_configs(configPath);
    require(configurations.size() == 1,
            "Config loader should return the configured experiment");
    require(configurations[0].graph.seed == 41 &&
                configurations[0].graph.blockageSeed == 73,
            "Config loader should preserve both reproducibility seeds");

    const ctp::ExperimentResult result =
        ctp::run_experiment(configurations[0].graph);
    require(result.plan.has_value(),
            "A triangle graph should have recovery routes for every edge");
    require(result.plan->paths.size() == result.plan->initialPath.size(),
            "Offline output should contain every path vertex and terminal entry");
    require(result.graphGenerationMicroseconds >= 0.0 &&
                result.preprocessingMicroseconds >= 0.0,
            "Experiment timings should be measured");

    ctp::run_experiment_batch(
        configurations,
        "build/test-results.csv",
        "build/test-graphs",
        "build/test-dots");
    const std::string results = read_file("build/test-results.csv");
    require(results.find("algorithm_paths") != std::string::npos &&
                results.find("scenario_metrics") != std::string::npos &&
                results.find(",ok,") != std::string::npos &&
                results.find("\"[[") != std::string::npos,
            "Results CSV should include a successful serialized offline plan");

    const ctp::GraphInstance saved =
        ctp::load_graph_instance("build/test-graphs/run_0001.ctp");
    require(saved.config.seed == 41 && saved.config.blockageSeed == 73,
            "Saved graph should preserve edge and blockage seeds");
    const std::string dot = read_file("build/test-dots/run_0001.dot");
    require(dot.find("color=\"red\"") != std::string::npos,
            "Experiment DOT output should highlight the blocked edge");
}

}  // namespace

int main() {
    try {
        test_configured_experiment_batch();
    } catch (const std::exception& error) {
        std::cerr << "Experiment test failure: " << error.what() << '\n';
        return 1;
    }
    std::cout << "Experiment tests passed\n";
    return 0;
}
