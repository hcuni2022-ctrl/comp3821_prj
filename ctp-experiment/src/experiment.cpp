// Runs offline Backtrack experiments, records timing, and saves graph artifacts.
#include "experiment.hpp"

#include "graph_generator.hpp"
#include "graph_utils.hpp"
#include "logger.hpp"
#include "visualizer.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ctp {
namespace {

using Clock = std::chrono::steady_clock;

double elapsed_microseconds(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::micro>(end - start).count();
}

bool same_edge(EdgeEndpoints a, EdgeEndpoints b) {
    return (a.u == b.u && a.v == b.v) || (a.u == b.v && a.v == b.u);
}

std::string artifact_name(std::size_t runId, const char* extension) {
    std::ostringstream name;
    name << "run_" << std::setw(4) << std::setfill('0') << runId << extension;
    return name.str();
}

}  // namespace

ExperimentResult run_experiment(const GraphConfig& config) {
    ExperimentResult result;

    const Clock::time_point generationStart = Clock::now();
    result.instance = generate_instance(config);
    result.graphGenerationMicroseconds =
        elapsed_microseconds(generationStart, Clock::now());

    const Clock::time_point preprocessingStart = Clock::now();
    try {
        result.plan = compute_backtrack_plan(
            result.instance.graph,
            result.instance.source,
            result.instance.destination);
    } catch (const InfeasibleRecoveryScenario& error) {
        result.preprocessingMicroseconds =
            elapsed_microseconds(preprocessingStart, Clock::now());
        result.initialPath = error.initial_path();
        result.initialPathCost = error.initial_path_cost();
        result.infeasibleEdge = error.blocked_edge();
        result.error = error.what();
        return result;
    }
    result.preprocessingMicroseconds =
        elapsed_microseconds(preprocessingStart, Clock::now());

    result.initialPath = result.plan->initialPath;
    result.initialPathCost = result.plan->initialPathCost;
    result.planStorageBytes = result.plan->estimatedStorageBytes;

    const Clock::time_point lookupStart = Clock::now();
    const auto actualScenario = std::find_if(
        result.plan->scenarios.begin(),
        result.plan->scenarios.end(),
        [&result](const BacktrackScenario& scenario) {
            return same_edge(scenario.blockedEdge, result.instance.blockedEdge);
        });
    if (actualScenario == result.plan->scenarios.end()) {
        result.actualRecoveryCost = 0.0;
        result.actualTotalTravelCost = result.plan->initialPathCost;
    } else {
        const std::vector<int>& recovery =
            result.plan->paths[actualScenario->pathIndex];
        if (recovery.empty() ||
            recovery.front() !=
                result.plan->initialPath[actualScenario->pathIndex]) {
            throw std::logic_error("Precomputed recovery path has invalid format");
        }
        result.actualRecoveryCost = actualScenario->recoveryCost;
        result.actualTotalTravelCost = actualScenario->totalTravelCost;
    }
    result.recoveryLookupMicroseconds =
        elapsed_microseconds(lookupStart, Clock::now());

    double worstTotal = result.plan->initialPathCost;
    double worstRatio = 1.0;
    for (const BacktrackScenario& scenario : result.plan->scenarios) {
        worstTotal = std::max(worstTotal, scenario.totalTravelCost);
        worstRatio = std::max(worstRatio, scenario.competitiveRatio);
    }
    result.worstCaseTotalTravelCost = worstTotal;
    result.worstCaseCompetitiveRatio = worstRatio;
    return result;
}

void run_experiment_batch(
    const std::vector<ExperimentConfig>& configurations,
    const std::string& resultsPath,
    const std::string& graphDirectory,
    const std::string& dotDirectory) {
    if (configurations.empty()) {
        throw std::invalid_argument("No experiment configurations were provided");
    }

    const std::filesystem::path resultFile(resultsPath);
    if (!resultFile.parent_path().empty()) {
        std::filesystem::create_directories(resultFile.parent_path());
    }
    std::filesystem::create_directories(graphDirectory);
    std::filesystem::create_directories(dotDirectory);
    std::ofstream results(resultsPath);
    if (!results) {
        throw std::runtime_error("Cannot open results file: " + resultsPath);
    }
    write_results_header(results);

    std::size_t runId = 0;
    for (const ExperimentConfig& configured : configurations) {
        for (std::size_t repetition = 0;
             repetition < configured.repetitions;
             ++repetition) {
            if (repetition >
                    std::numeric_limits<std::uint64_t>::max() -
                        configured.graph.seed ||
                repetition >
                    std::numeric_limits<std::uint64_t>::max() -
                        configured.graph.blockageSeed.value_or(
                            configured.graph.seed)) {
                throw std::overflow_error("Experiment repetition seed overflow");
            }

            GraphConfig graphConfig = configured.graph;
            graphConfig.seed += repetition;
            graphConfig.blockageSeed =
                configured.graph.blockageSeed.value_or(configured.graph.seed) +
                repetition;
            ++runId;

            ExperimentResult result = run_experiment(graphConfig);
            const std::string fileName = artifact_name(runId, ".ctp");
            const std::filesystem::path graphFile =
                std::filesystem::path(graphDirectory) / fileName;
            const std::filesystem::path dotFile =
                std::filesystem::path(dotDirectory) /
                artifact_name(runId, ".dot");

            save_graph_instance(result.instance, graphFile.string());
            save_graph_dot(
                result.instance, result.initialPath, dotFile.string());
            write_result_row(
                results,
                runId,
                result,
                graphFile.string(),
                dotFile.string());
            results.flush();
            if (!results) {
                throw std::runtime_error(
                    "Failed while writing results file: " + resultsPath);
            }

            std::cout << "Run " << runId << ": "
                      << (result.plan ? "ok" : "infeasible");
            if (!result.plan) {
                std::cout << " (" << result.error << ')';
            }
            std::cout << '\n';
        }
    }
}

}  // namespace ctp
