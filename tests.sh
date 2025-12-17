#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"

cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --target unit_tests
ctest --test-dir "${BUILD_DIR}" --output-on-failure -R unit_tests
