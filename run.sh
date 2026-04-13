#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"

# Multi-config generators (MSVC) put the binary in a config subdirectory;
# single-config generators (Make/Ninja) put it directly in the build folder.
if [ -f "$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-tests.exe" ]; then
    TEST_BIN="$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-tests.exe"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-tests" ]; then
    TEST_BIN="$SCRIPT_DIR/$BUILD_DIR/$BUILD_TYPE/secs-tests"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/secs-tests.exe" ]; then
    TEST_BIN="$SCRIPT_DIR/$BUILD_DIR/secs-tests.exe"
elif [ -f "$SCRIPT_DIR/$BUILD_DIR/secs-tests" ]; then
    TEST_BIN="$SCRIPT_DIR/$BUILD_DIR/secs-tests"
else
    echo "Error: secs-tests binary not found. Run build.sh first."
    exit 1
fi

echo "Running tests: $TEST_BIN"
"$TEST_BIN" --gtest_color=yes

