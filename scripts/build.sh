#!/usr/bin/env bash

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="${2:-build}"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_PATH="$ROOT/$BUILD_DIR"

echo "Installing dependencies via Conan..."
conan install "$ROOT" --output-folder="$BUILD_PATH" --build=missing -s build_type="$BUILD_TYPE" || exit 1

echo "Configuring secs ($BUILD_TYPE)..."
cmake -S "$ROOT" -B "$BUILD_PATH" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_TOOLCHAIN_FILE="$BUILD_PATH/conan_toolchain.cmake" -DCMAKE_PREFIX_PATH="$BUILD_PATH" || exit 1

echo "Building secs..."
cmake --build "$BUILD_PATH" --config "$BUILD_TYPE" || exit 1

echo "Build complete: $BUILD_PATH"
