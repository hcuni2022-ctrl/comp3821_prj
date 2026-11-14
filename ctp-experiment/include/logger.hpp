// Declares CSV result serialization for experiment runs.
#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

namespace ctp {

struct ExperimentResult;

void write_results_header(std::ostream& output);
void write_result_row(
    std::ostream& output,
    std::size_t runId,
    const ExperimentResult& result,
    const std::string& graphPath,
    const std::string& dotPath);

}  // namespace ctp
