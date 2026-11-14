// Parses and validates experiment batch settings from a CSV file.
#include "config.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ctp {
namespace {

std::string trim(std::string value) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return {};
    }
    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

std::vector<std::string> split_csv_row(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream input(line);
    std::string field;
    while (std::getline(input, field, ',')) {
        fields.push_back(trim(field));
    }
    if (!line.empty() && line.back() == ',') {
        fields.emplace_back();
    }
    return fields;
}

template <typename Value>
Value parse_value(const std::string& text, const std::string& fieldName) {
    if constexpr (std::is_unsigned_v<Value>) {
        if (!text.empty() && text.front() == '-') {
            throw std::runtime_error(fieldName + " must not be negative");
        }
    }
    std::istringstream input(text);
    Value value{};
    if (!(input >> value)) {
        throw std::runtime_error("Invalid " + fieldName + ": " + text);
    }
    input >> std::ws;
    if (!input.eof()) {
        throw std::runtime_error("Unexpected trailing data in " + fieldName);
    }
    return value;
}

std::size_t required_column(
    const std::map<std::string, std::size_t>& columns,
    const std::string& name) {
    const auto column = columns.find(name);
    if (column == columns.end()) {
        throw std::runtime_error("Missing experiment CSV column: " + name);
    }
    return column->second;
}

}  // namespace

std::vector<ExperimentConfig> load_experiment_configs(
    const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open experiment config: " + path);
    }

    std::string line;
    std::size_t lineNumber = 0;
    std::map<std::string, std::size_t> columns;
    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string cleaned = trim(line);
        if (cleaned.empty() || cleaned.front() == '#') {
            continue;
        }
        const std::vector<std::string> names = split_csv_row(cleaned);
        for (std::size_t index = 0; index < names.size(); ++index) {
            if (names[index].empty() ||
                !columns.emplace(names[index], index).second) {
                throw std::runtime_error(
                    "Empty or duplicate experiment CSV column");
            }
        }
        break;
    }
    if (columns.empty()) {
        throw std::runtime_error("Experiment config has no CSV header");
    }

    const std::size_t vertexColumn = required_column(columns, "vertex_count");
    const std::size_t probabilityColumn =
        required_column(columns, "edge_probability");
    const std::size_t minWeightColumn = required_column(columns, "min_weight");
    const std::size_t maxWeightColumn = required_column(columns, "max_weight");
    const std::size_t edgeSeedColumn = required_column(columns, "edge_seed");
    const std::size_t blockageSeedColumn =
        required_column(columns, "blockage_seed");
    const std::size_t repetitionsColumn =
        required_column(columns, "repetitions");
    const std::size_t fieldCount = columns.size();

    std::vector<ExperimentConfig> configurations;
    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string cleaned = trim(line);
        if (cleaned.empty() || cleaned.front() == '#') {
            continue;
        }
        const std::vector<std::string> fields = split_csv_row(cleaned);
        if (fields.size() != fieldCount) {
            throw std::runtime_error(
                "Wrong field count on experiment CSV line " +
                std::to_string(lineNumber));
        }

        try {
            ExperimentConfig config;
            config.graph.vertexCount =
                parse_value<std::size_t>(fields[vertexColumn], "vertex_count");
            config.graph.edgeProbability = parse_value<double>(
                fields[probabilityColumn], "edge_probability");
            config.graph.minWeight =
                parse_value<EdgeWeight>(fields[minWeightColumn], "min_weight");
            config.graph.maxWeight =
                parse_value<EdgeWeight>(fields[maxWeightColumn], "max_weight");
            config.graph.seed =
                parse_value<std::uint64_t>(fields[edgeSeedColumn], "edge_seed");
            config.graph.blockageSeed = parse_value<std::uint64_t>(
                fields[blockageSeedColumn], "blockage_seed");
            config.repetitions =
                parse_value<std::size_t>(fields[repetitionsColumn], "repetitions");

            if (config.graph.vertexCount < 3 ||
                !std::isfinite(config.graph.edgeProbability) ||
                config.graph.edgeProbability < 0.0 ||
                config.graph.edgeProbability > 1.0 ||
                config.graph.minWeight <= 0 ||
                config.graph.maxWeight < config.graph.minWeight ||
                config.repetitions == 0) {
                throw std::runtime_error("Configuration values are out of range");
            }
            configurations.push_back(config);
        } catch (const std::exception& error) {
            throw std::runtime_error(
                "Invalid experiment CSV line " + std::to_string(lineNumber) +
                ": " + error.what());
        }
    }
    if (!input.eof()) {
        throw std::runtime_error("Failed while reading experiment config: " + path);
    }
    if (configurations.empty()) {
        throw std::runtime_error("Experiment config has no data rows");
    }
    return configurations;
}

}  // namespace ctp
