#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
EXEC="lock_free_data_structures"

if [[ ! -x "$BUILD_DIR/$EXEC" ]]; then
  echo "Building executable..."
  cmake -S . -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" -j
fi

# Argument Handling
# understand how to write bash conditionals
if [[ $# -eq 0 ]]; then
  echo "No arguments provided, defaulting to 'stress 1 10000'"
  set -- stress 1 10000
else
  echo "Arguments provided: $*"
fi

if [[ "${1:-}" == "--gdb" ]]; then
    shift
    echo "Running under gdb with args: $*"
    exec gdb --args "$BUILD_DIR/$EXEC" "$@"
fi

echo "Running: $EXEC $*"
exec "$BUILD_DIR/$EXEC" "$@"