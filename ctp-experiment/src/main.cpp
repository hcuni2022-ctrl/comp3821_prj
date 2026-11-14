// Provides commands for graph generation, validation, and experiment batches.
#include "config.hpp"
#include "experiment.hpp"
#include "graph_generator.hpp"
#include "graph_utils.hpp"

#include <boost/graph/graph_traits.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace {

template <typename Value>
Value parse_number(const std::string& text, const std::string& option) {
    if constexpr (std::is_unsigned_v<Value>) {
        if (!text.empty() && text.front() == '-') {
            throw std::invalid_argument(option + " must not be negative");
        }
    }

    std::istringstream input(text);
    Value value{};
    if (!(input >> value)) {
        throw std::invalid_argument("Invalid value for " + option);
    }
    input >> std::ws;
    if (!input.eof()) {
        throw std::invalid_argument("Unexpected trailing data for " + option);
    }
    return value;
}

std::map<std::string, std::string> parse_options(
    int argc,
    char* argv[],
    int firstOption) {
    std::map<std::string, std::string> options;
    for (int index = firstOption; index < argc; index += 2) {
        const std::string option = argv[index];
        if (option.rfind("--", 0) != 0 || index + 1 >= argc) {
            throw std::invalid_argument("Expected an option followed by a value");
        }
        if (!options.emplace(option, argv[index + 1]).second) {
            throw std::invalid_argument("Duplicate option: " + option);
        }
    }
    return options;
}

const std::string& required_option(
    const std::map<std::string, std::string>& options,
    const std::string& name) {
    const auto value = options.find(name);
    if (value == options.end()) {
        throw std::invalid_argument("Missing required option: " + name);
    }
    return value->second;
}

void reject_unknown_options(
    const std::map<std::string, std::string>& options,
    const std::vector<std::string>& allowed) {
    for (const auto& option : options) {
        if (std::find(allowed.begin(), allowed.end(), option.first) ==
            allowed.end()) {
            throw std::invalid_argument("Unknown option: " + option.first);
        }
    }
}

void print_usage(std::ostream& output) {
    output
        << "Usage:\n"
        << "  ctp generate --vertices N --edge-probability P --min-weight W "
           "--max-weight W --seed S [--blockage-seed S] --output FILE\n"
        << "  ctp validate FILE\n"
        << "  ctp run --config FILE --results FILE --graphs DIR --dots DIR\n";
}

void print_instance_summary(const ctp::GraphInstance& instance) {
    std::cout << "vertices=" << boost::num_vertices(instance.graph)
              << " edges=" << boost::num_edges(instance.graph)
              << " source=" << instance.source
              << " destination=" << instance.destination
              << " blocked_edge=" << instance.blockedEdge.u << ','
              << instance.blockedEdge.v << " edge_seed=" << instance.config.seed
              << " blockage_seed="
              << instance.config.blockageSeed.value_or(instance.config.seed)
              << '\n';
}

void generate(int argc, char* argv[]) {
    const auto options = parse_options(argc, argv, 2);
    reject_unknown_options(
        options,
        {"--vertices",
         "--edge-probability",
         "--min-weight",
         "--max-weight",
         "--seed",
         "--blockage-seed",
         "--output"});

    ctp::GraphConfig config;
    config.vertexCount =
        parse_number<std::size_t>(required_option(options, "--vertices"), "--vertices");
    config.edgeProbability = parse_number<double>(
        required_option(options, "--edge-probability"), "--edge-probability");
    config.minWeight = parse_number<ctp::EdgeWeight>(
        required_option(options, "--min-weight"), "--min-weight");
    config.maxWeight = parse_number<ctp::EdgeWeight>(
        required_option(options, "--max-weight"), "--max-weight");
    config.seed =
        parse_number<std::uint64_t>(required_option(options, "--seed"), "--seed");
    const auto blockageSeed = options.find("--blockage-seed");
    if (blockageSeed != options.end()) {
        config.blockageSeed =
            parse_number<std::uint64_t>(blockageSeed->second, "--blockage-seed");
    }
    const std::filesystem::path outputPath(
        required_option(options, "--output"));

    const ctp::GraphInstance instance = ctp::generate_instance(config);
    if (!outputPath.parent_path().empty()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }
    ctp::save_graph_instance(instance, outputPath.string());
    std::cout << "Saved " << outputPath.string() << '\n';
    print_instance_summary(instance);
}

void validate(const std::string& path) {
    const ctp::GraphInstance instance = ctp::load_graph_instance(path);
    std::cout << "Valid graph instance: " << path << '\n';
    print_instance_summary(instance);
}

void run_batch(int argc, char* argv[]) {
    const auto options = parse_options(argc, argv, 2);
    reject_unknown_options(options, {"--config", "--results", "--graphs", "--dots"});
    const auto configurations =
        ctp::load_experiment_configs(required_option(options, "--config"));
    ctp::run_experiment_batch(
        configurations,
        required_option(options, "--results"),
        required_option(options, "--graphs"),
        required_option(options, "--dots"));
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc < 2 || std::string(argv[1]) == "--help") {
            print_usage(argc < 2 ? std::cerr : std::cout);
            return argc < 2 ? 2 : 0;
        }

        const std::string command = argv[1];
        if (command == "generate") {
            generate(argc, argv);
            return 0;
        }
        if (command == "validate") {
            if (argc != 3) {
                throw std::invalid_argument("validate requires exactly one file path");
            }
            validate(argv[2]);
            return 0;
        }
        if (command == "run") {
            run_batch(argc, argv);
            return 0;
        }

        print_usage(std::cerr);
        throw std::invalid_argument("Unknown command: " + command);
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
