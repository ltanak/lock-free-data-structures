#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
EXEC="lock_free_data_structures"

if [[ ! -x "$BUILD_DIR/$EXEC" ]]; then
  echo "Building executable..."
  cmake -S . -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" -j
fi

exec "$BUILD_DIR/$EXEC" "$@"