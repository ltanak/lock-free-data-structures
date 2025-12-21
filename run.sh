#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
EXEC="lock_free_data_structures"

if [[ ! -x "$BUILD_DIR/$EXEC" ]]; then
  echo "Building executable..."
  cmake -S . -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" -j
fi

SEED=""
GDB=0
VALGRIND=0
PERF=0
POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --seed=*)
      SEED="${1#*=}"
      shift
      ;;
    --gdb)
      GDB=1
      shift
      ;;
    --valgrind)
      VALGRIND=1
      shift
      ;;
    --perf)
      PERF=1
      shift
      ;;
    --help|-h)
      echo "Usage:"
      echo "  ./run.sh [OPTIONS] <mode> <threads> <operations>"
      echo
      echo "Options:"
      echo "  --seed=UINT64     Explicit pseudo-random seed"
      echo "  --gdb             Run under gdb"
      echo "  --valgrind        Run under valgrind (memcheck)"
      echo "  --perf            Run under perf record"
      echo
      echo "Mode:"
      echo "  stress            Stress testing lock-free datastructure"
      echo "  order             Order-preservation testing lock-free datastructure"
      echo
      echo "Threads:"
      echo "  The amount of producer / consumer threads you want the datastructure to run on"
      echo "  NOTE: running multiple threads on a non-multi producer / consumer will cause unexpected results"
      echo
      echo "Operations:"
      echo "  The integer number of orders to run through the simulation"
      echo
      echo "Examples:"
      echo "  ./run.sh stress 1 1000"
      echo "  ./run.sh --seed=42 order 4 100000"
      echo "  ./run.sh --perf stress 8 1000000"
      echo "  ./run.sh --valgrind order 1 1000"
      echo "  ./run.sh --gdb --seed=123 stress 1 1000"
      exit 0
      ;;
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

ARGS=("$@")

if [[ -n "$SEED" ]]; then
  ARGS+=(--seed "$SEED")
fi

CMD=("$BUILD_DIR/$EXEC" "${ARGS[@]}")

# Tools

if [[ $PERF -eq 1 ]]; then
  CMD=(perf record -g -- "${CMD[@]}")
fi

if [[ $VALGRIND -eq 1 ]]; then
  CMD=(valgrind --tool=memcheck --leak-check=full --track-origins=yes "${CMD[@]}")
fi

# Running code

if [[ $GDB -eq 1 ]]; then
  echo "Running under gdb with command: ${CMD[*]}"
  exec gdb --args "${CMD[@]}"
else
  exec "${CMD[@]}"
fi

# # Argument Handling
# # understand how to write bash conditionals
# if [[ $# -eq 0 ]]; then
#   # echo "No arguments provided, defaulting to 'stress 1 10000'"
#   set -- stress 1 10000
# # else
# #   echo "Arguments provided: $*"
# fi