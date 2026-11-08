#!/usr/bin/env bash
# Builds the test binary and runs only the audit regression tests
# (src/gtests/AuditTests.cpp, suites named Audit*).
#
# Usage: scripts/run-audit-tests.sh [BuildType] [BuildDir] [extra gtest args...]
#   e.g. scripts/run-audit-tests.sh Debug build --gtest_filter='Audit04*'
#
# Use a Debug build: some audit tests rely on Debug bounds checks to detect
# out-of-bounds reads and may pass by accident in Release.
#
# Requires dependencies to be installed already (run scripts/build-all.sh once).

set -euo pipefail

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"
shift $(( $# > 2 ? 2 : $# ))

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_PATH="$ROOT/$BUILD_DIR"

if [ ! -f "$BUILD_PATH/conan_toolchain.cmake" ]; then
    echo "Error: $BUILD_PATH/conan_toolchain.cmake not found. Run scripts/build-all.sh first." >&2
    exit 1
fi

echo "==> Configuring secs ($BUILD_TYPE) with tests..."
cmake -S "$ROOT" -B "$BUILD_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$BUILD_PATH/conan_toolchain.cmake" \
    -DCMAKE_PREFIX_PATH="$BUILD_PATH" \
    -DBUILD_GTESTS=ON > /dev/null

echo "==> Building secs-tests..."
cmake --build "$BUILD_PATH" --config "$BUILD_TYPE" --target secs-tests

# Multi-config generators (MSVC) put the binary in a config subdirectory;
# single-config generators (Make/Ninja) put it directly in the build folder.
TEST_BIN=""
for candidate in \
    "$BUILD_PATH/$BUILD_TYPE/secs-tests.exe" \
    "$BUILD_PATH/$BUILD_TYPE/secs-tests" \
    "$BUILD_PATH/secs-tests.exe" \
    "$BUILD_PATH/secs-tests"; do
    if [ -f "$candidate" ]; then TEST_BIN="$candidate"; break; fi
done

if [ -z "$TEST_BIN" ]; then
    echo "Error: secs-tests binary not found in $BUILD_PATH." >&2
    exit 1
fi

# Default to all audit suites; a --gtest_filter passed by the caller overrides it.
FILTER="--gtest_filter=Audit*:*/Audit*"
for arg in "$@"; do
    case "$arg" in --gtest_filter=*) FILTER="";; esac
done

echo "==> Running audit tests: $TEST_BIN"
"$TEST_BIN" ${FILTER:+"$FILTER"} --gtest_color=yes "$@"
