#!/usr/bin/env bash
# Builds everything: the secs library, the unit tests and the benchmarks.
#
# Usage: scripts/build-all.sh [BuildType] [BuildDir]
#   BuildType  Debug (default) | Release | RelWithDebInfo | MinSizeRel
#   BuildDir   build directory, relative to the repo root (default: build)

set -euo pipefail

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_PATH="$ROOT/$BUILD_DIR"

echo "==> Installing dependencies via Conan ($BUILD_TYPE)..."
conan install "$ROOT" --output-folder="$BUILD_PATH" --build=missing -s build_type="$BUILD_TYPE"

echo "==> Configuring secs ($BUILD_TYPE) with tests and benchmarks..."
cmake -S "$ROOT" -B "$BUILD_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$BUILD_PATH/conan_toolchain.cmake" \
    -DCMAKE_PREFIX_PATH="$BUILD_PATH" \
    -DBUILD_GTESTS=ON \
    -DBUILD_BENCHMARKS=ON

echo "==> Building all targets..."
cmake --build "$BUILD_PATH" --config "$BUILD_TYPE"

echo "==> Build complete: $BUILD_PATH"
