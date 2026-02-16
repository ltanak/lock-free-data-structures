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
CACHEGRIND=0
RUN_ALL=0
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
    --cachegrind)
      CACHEGRIND=1
      shift
      ;;
    --perf)
      PERF=1
      shift
      ;;
    --all)
      RUN_ALL=1
      shift
      ;;
    --help|-h)
      echo "Usage:"
      echo "  ./run.sh [OPTIONS] <mode> <threads> <operations>"
      echo "  ./run.sh [OPTIONS] --all <threads> <operations>"
      echo
      echo "Options:"
      echo "  --seed=UINT64     Explicit pseudo-random seed"
      echo "  --gdb             Run under gdb"
      echo "  --valgrind        Run under valgrind (memcheck)"
      echo "  --perf            Run under perf record"
      echo "  --cachegrind      Run under cachegrind"
      echo "  --all             Run all test types (stress, order) with the same run ID"
      echo
      echo "Mode:"
      echo "  stress            Stress testing lock-free datastructure"
      echo "  order             Order-preservation testing lock-free datastructure"
      echo "  --all             Run both stress and order tests"
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
      echo "  ./run.sh --all 4 10000"
      echo "  ./run.sh --seed=42 --all 4 100000"
      exit 0
      ;;
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

# Generate a unique run ID (random number)
RUN_ID="$RANDOM$RANDOM$RANDOM"

# Handle --all flag
if [[ $RUN_ALL -eq 1 ]]; then
  if [[ ${#POSITIONAL[@]} -lt 2 ]]; then
    echo "Error: --all requires <threads> <operations>"
    echo "Usage: ./run.sh --all <threads> <operations>"
    exit 1
  fi

  THREADS="${POSITIONAL[0]}"
  OPERATIONS="${POSITIONAL[1]}"
  
  echo "=========================================="
  echo "Running ALL tests with Run ID: $RUN_ID"
  echo "Threads: $THREADS, Operations: $OPERATIONS"
  if [[ -n "$SEED" ]]; then
    echo "Seed: $SEED"
  fi
  echo "=========================================="
  echo
  
  for MODE in stress order; do
    echo "Running $MODE test"
    echo "===================="
    
    ARGS=("$MODE" "$THREADS" "$OPERATIONS")
    
    if [[ -n "$SEED" ]]; then
      ARGS+=(--seed "$SEED")
    fi
    
    ARGS+=(--run-id "$RUN_ID")
    
    CMD=("$BUILD_DIR/$EXEC" "${ARGS[@]}")
    
    # Tools
    if [[ $PERF -eq 1 ]]; then
      CMD=(perf record -g -- "${CMD[@]}")
    fi
    
    if [[ $VALGRIND -eq 1 ]]; then
      CMD=(valgrind --tool=memcheck --leak-check=full --track-origins=yes "${CMD[@]}")
    fi
    
    if [[ $CACHEGRIND -eq 1 ]]; then
      CMD=(valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes "${CMD[@]}")
    fi
    
    # Run the command
    if [[ $GDB -eq 1 ]]; then
      echo "Error: Cannot use --gdb with --all flag"
      exit 1
    else
      "${CMD[@]}"
      EXIT_CODE=$?
      
      if [[ $EXIT_CODE -ne 0 ]]; then
        echo "Error: $MODE test failed with exit code $EXIT_CODE"
        exit $EXIT_CODE
      fi
    fi
    
    echo
  done
  
  echo "=========================================="
  echo "All tests completed with Run ID: $RUN_ID"
  echo "=========================================="
  exit 0
fi

# Original single-test execution
ARGS=("$@")

if [[ -n "$SEED" ]]; then
  ARGS+=(--seed "$SEED")
fi

ARGS+=(--run-id "$RUN_ID")

CMD=("$BUILD_DIR/$EXEC" "${ARGS[@]}")

# Tools

if [[ $PERF -eq 1 ]]; then
  CMD=(perf record -g -- "${CMD[@]}")
fi

if [[ $VALGRIND -eq 1 ]]; then
  CMD=(valgrind --tool=memcheck --leak-check=full --track-origins=yes "${CMD[@]}")
fi


if [[ $CACHEGRIND -eq 1 ]]; then
  CMD=(valgrind --tool=cachegrind --cache-sim=yes --branch-sim=yes "${CMD[@]}")
fi
# --tool=cachegrind ls -l

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