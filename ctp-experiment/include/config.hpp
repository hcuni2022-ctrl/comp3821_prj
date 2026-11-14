// Declares experiment-batch settings loaded from the CSV configuration file.
#pragma once

#include "graph.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ctp {

struct ExperimentConfig {
    GraphConfig graph;
    std::size_t repetitions = 1;
};

std::vector<ExperimentConfig> load_experiment_configs(
    const std::string& path);

}  // namespace ctp
