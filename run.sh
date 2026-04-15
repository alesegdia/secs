#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"
BUILD_PATH="$SCRIPT_DIR/$BUILD_DIR"

echo "Configuring secs with tests ($BUILD_TYPE)..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$BUILD_PATH/conan_toolchain.cmake" \
    -DCMAKE_PREFIX_PATH="$BUILD_PATH" \
    -DBUILD_GTESTS=ON || exit 1

echo "Building secs-tests..."
cmake --build "$BUILD_PATH" --config "$BUILD_TYPE" --target secs-tests || exit 1

# Multi-config generators (MSVC) put the binary in a config subdirectory;
# single-config generators (Make/Ninja) put it directly in the build folder.
if [ -f "$BUILD_PATH/$BUILD_TYPE/secs-tests.exe" ]; then
    TEST_BIN="$BUILD_PATH/$BUILD_TYPE/secs-tests.exe"
elif [ -f "$BUILD_PATH/$BUILD_TYPE/secs-tests" ]; then
    TEST_BIN="$BUILD_PATH/$BUILD_TYPE/secs-tests"
elif [ -f "$BUILD_PATH/secs-tests.exe" ]; then
    TEST_BIN="$BUILD_PATH/secs-tests.exe"
elif [ -f "$BUILD_PATH/secs-tests" ]; then
    TEST_BIN="$BUILD_PATH/secs-tests"
else
    echo "Error: secs-tests binary not found."
    exit 1
fi

echo "Running tests: $TEST_BIN"
"$TEST_BIN" --gtest_color=yes

