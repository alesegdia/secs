#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"
BUILD_PATH="$SCRIPT_DIR/$BUILD_DIR"

echo "Configuring secs with benchmarks ($BUILD_TYPE)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$BUILD_PATH/conan_toolchain.cmake" \
    -DCMAKE_PREFIX_PATH="$BUILD_PATH" \
    -DBUILD_BENCHMARKS=ON || exit 1

echo "Building secs-benchmarks..."
cmake --build "$BUILD_PATH" --config "$BUILD_TYPE" --target secs-benchmarks || exit 1

# Locate the benchmark binary — handles both MSVC multi-config and single-config generators.
if [ -f "$BUILD_PATH/$BUILD_TYPE/secs-benchmarks.exe" ]; then
    BENCH_BIN="$BUILD_PATH/$BUILD_TYPE/secs-benchmarks.exe"
elif [ -f "$BUILD_PATH/$BUILD_TYPE/secs-benchmarks" ]; then
    BENCH_BIN="$BUILD_PATH/$BUILD_TYPE/secs-benchmarks"
elif [ -f "$BUILD_PATH/secs-benchmarks.exe" ]; then
    BENCH_BIN="$BUILD_PATH/secs-benchmarks.exe"
elif [ -f "$BUILD_PATH/secs-benchmarks" ]; then
    BENCH_BIN="$BUILD_PATH/secs-benchmarks"
else
    echo "Error: secs-benchmarks binary not found."
    exit 1
fi

echo "Running benchmarks: $BENCH_BIN"
echo ""
"$BENCH_BIN" --benchmark_format=console "${@:3}"
