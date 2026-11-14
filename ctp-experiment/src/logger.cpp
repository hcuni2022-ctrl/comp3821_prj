// Serializes experiment paths and measurements into a CSV results file.
#include "logger.hpp"

#include "experiment.hpp"

#include <boost/graph/graph_traits.hpp>

#include <iomanip>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace ctp {
namespace {

std::string csv_escape(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) {
        return value;
    }
    std::string escaped = "\"";
    for (char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        } else {
            escaped += character;
        }
    }
    escaped += '"';
    return escaped;
}

std::string path_json(const std::vector<int>& path) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0; index < path.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        output << path[index];
    }
    output << ']';
    return output.str();
}

std::string paths_json(const std::vector<std::vector<int>>& paths) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0; index < paths.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        output << path_json(paths[index]);
    }
    output << ']';
    return output.str();
}

template <typename Value>
std::string number_string(Value value) {
    std::ostringstream output;
    output << std::setprecision(17) << value;
    return output.str();
}

std::string scenarios_json(const BacktrackPlan& plan) {
    std::ostringstream output;
    output << '[';
    for (std::size_t index = 0; index < plan.scenarios.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        const BacktrackScenario& scenario = plan.scenarios[index];
        output << "{\"path_index\":" << scenario.pathIndex
               << ",\"blocked_edge\":[" << scenario.blockedEdge.u << ','
               << scenario.blockedEdge.v << ']'
               << ",\"initial_prefix_cost\":"
               << number_string(scenario.initialPrefixCost)
               << ",\"backtrack_cost\":" << number_string(scenario.backtrackCost)
               << ",\"reroute_cost\":" << number_string(scenario.rerouteCost)
               << ",\"recovery_cost\":" << number_string(scenario.recoveryCost)
               << ",\"total_travel_cost\":"
               << number_string(scenario.totalTravelCost)
               << ",\"offline_cost\":" << number_string(scenario.offlineCost)
               << ",\"competitive_ratio\":"
               << number_string(scenario.competitiveRatio) << '}';
    }
    output << ']';
    return output.str();
}

template <typename Value>
std::string optional_number(const std::optional<Value>& value) {
    return value ? number_string(*value) : std::string{};
}

}  // namespace

void write_results_header(std::ostream& output) {
    output
        << "run_id,status,vertex_count,edge_count,edge_probability,min_weight,"
           "max_weight,edge_seed,blockage_seed,source,destination,blocked_u,"
           "blocked_v,initial_path,initial_path_cost,algorithm_paths,"
           "scenario_metrics,graph_generation_us,"
           "preprocessing_us,recovery_lookup_us,actual_recovery_cost,"
           "actual_total_travel_cost,worst_case_total_travel_cost,"
           "worst_case_competitive_ratio,plan_storage_bytes,infeasible_u,"
           "infeasible_v,error,graph_file,dot_file\n";
}

void write_result_row(
    std::ostream& output,
    std::size_t runId,
    const ExperimentResult& result,
    const std::string& graphPath,
    const std::string& dotPath) {
    const GraphInstance& instance = result.instance;
    const bool feasible = result.plan.has_value();
    const std::string algorithmPaths =
        feasible ? paths_json(result.plan->paths) : std::string{};
    const std::string scenarioMetrics =
        feasible ? scenarios_json(*result.plan) : std::string{};
    const std::string initialPath = path_json(result.initialPath);
    const std::string infeasibleU =
        result.infeasibleEdge ? number_string(result.infeasibleEdge->u) : "";
    const std::string infeasibleV =
        result.infeasibleEdge ? number_string(result.infeasibleEdge->v) : "";

    output << runId << ',' << (feasible ? "ok" : "infeasible") << ','
           << instance.config.vertexCount << ','
           << boost::num_edges(instance.graph) << ','
           << number_string(instance.config.edgeProbability) << ','
           << instance.config.minWeight << ',' << instance.config.maxWeight << ','
           << instance.config.seed << ','
           << instance.config.blockageSeed.value_or(instance.config.seed) << ','
           << instance.source << ',' << instance.destination << ','
           << instance.blockedEdge.u << ',' << instance.blockedEdge.v << ','
           << csv_escape(initialPath) << ','
           << (result.initialPath.empty()
                   ? std::string{}
                   : number_string(result.initialPathCost))
           << ',' << csv_escape(algorithmPaths) << ','
           << csv_escape(scenarioMetrics) << ','
           << number_string(result.graphGenerationMicroseconds) << ','
           << number_string(result.preprocessingMicroseconds) << ','
           << optional_number(result.recoveryLookupMicroseconds) << ','
           << optional_number(result.actualRecoveryCost) << ','
           << optional_number(result.actualTotalTravelCost) << ','
           << optional_number(result.worstCaseTotalTravelCost) << ','
           << optional_number(result.worstCaseCompetitiveRatio) << ','
           << result.planStorageBytes << ',' << infeasibleU << ',' << infeasibleV
           << ',' << csv_escape(result.error) << ',' << csv_escape(graphPath)
           << ',' << csv_escape(dotPath) << '\n';
}

}  // namespace ctp
