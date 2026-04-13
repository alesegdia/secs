#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"

# Locate the benchmark binary — handles both MSVC multi-config and single-config generators.
if [ -f "$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-benchmarks.exe" ]; then
    BENCH_BIN="$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-benchmarks.exe"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-benchmarks" ]; then
    BENCH_BIN="$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-benchmarks"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/secs-benchmarks.exe" ]; then
    BENCH_BIN="$SCRIPT_DIR/$BUILD_DIR/secs-benchmarks.exe"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/secs-benchmarks" ]; then
    BENCH_BIN="$SCRIPT_DIR/$BUILD_DIR/secs-benchmarks"
else
    echo "Error: secs-benchmarks binary not found. Run ./build.sh $BUILD_TYPE first."
    exit 1
fi

echo "Running benchmarks: $BENCH_BIN"
echo ""
"$BENCH_BIN" --benchmark_format=console "${@:3}"
