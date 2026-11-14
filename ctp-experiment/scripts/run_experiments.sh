#!/usr/bin/env bash
# Builds and runs the CSV-configured offline Backtrack experiment batch.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

make -C "$PROJECT_DIR"
"$PROJECT_DIR/build/ctp" run \
    --config "$PROJECT_DIR/config/experiment_config.csv" \
    --results "$PROJECT_DIR/data/results/results.csv" \
    --graphs "$PROJECT_DIR/data/generated" \
    --dots "$PROJECT_DIR/visualisations/dot"
