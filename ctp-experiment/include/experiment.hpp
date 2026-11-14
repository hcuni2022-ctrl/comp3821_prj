// Declares single-run measurements and the configured experiment-batch runner.
#pragma once

#include "baseline_algorithms.hpp"
#include "config.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ctp {

struct ExperimentResult {
    GraphInstance instance;
    std::vector<int> initialPath;
    double initialPathCost = 0.0;
    std::optional<BacktrackPlan> plan;
    std::optional<EdgeEndpoints> infeasibleEdge;
    std::string error;
    double graphGenerationMicroseconds = 0.0;
    double preprocessingMicroseconds = 0.0;
    std::optional<double> recoveryLookupMicroseconds;
    std::optional<double> actualRecoveryCost;
    std::optional<double> actualTotalTravelCost;
    std::optional<double> worstCaseTotalTravelCost;
    std::optional<double> worstCaseCompetitiveRatio;
    std::size_t planStorageBytes = 0;
};

ExperimentResult run_experiment(const GraphConfig& config);

void run_experiment_batch(
    const std::vector<ExperimentConfig>& configurations,
    const std::string& resultsPath,
    const std::string& graphDirectory,
    const std::string& dotDirectory);

}  // namespace ctp
