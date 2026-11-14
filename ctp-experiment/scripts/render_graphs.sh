#!/usr/bin/env bash
# Renders generated Graphviz DOT files into PNG images.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DOT_DIR="${1:-$PROJECT_DIR/visualisations/dot}"
IMAGE_DIR="${2:-$PROJECT_DIR/visualisations/images}"

if ! command -v dot >/dev/null 2>&1; then
    echo "Graphviz 'dot' is required to render graphs." >&2
    exit 1
fi

mkdir -p "$IMAGE_DIR"
shopt -s nullglob
dot_files=("$DOT_DIR"/*.dot)
if ((${#dot_files[@]} == 0)); then
    echo "No DOT files found in $DOT_DIR" >&2
    exit 1
fi

for dot_file in "${dot_files[@]}"; do
    base_name="$(basename "$dot_file" .dot)"
    dot -Tpng "$dot_file" -o "$IMAGE_DIR/$base_name.png"
done
