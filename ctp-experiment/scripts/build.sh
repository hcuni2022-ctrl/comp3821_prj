#!/usr/bin/env bash
# Builds the C++ experiment scaffold using the project Makefile.
set -euo pipefail

cd "$(dirname "$0")/.."
make
