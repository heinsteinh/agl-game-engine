#!/bin/bash

# AGL Demo Quick Build Script
# This script uses a pre-built gamelib to quickly build and test demo applications

set -e

# Configuration
DEMO_NAME=${1:-renderer}
BUILD_TYPE=${2:-Release}
GAMELIB_INSTALL_DIR=${3:-../gamelib/gamelib_install}

echo "=== AGL Demo Quick Build ==="
echo "Demo: $DEMO_NAME"
echo "Build Type: $BUILD_TYPE"
echo "GameLib Location: $GAMELIB_INSTALL_DIR"
echo ""

# Check if gamelib is built
if [ ! -f "$GAMELIB_INSTALL_DIR/lib/libagl_gamelib.a" ]; then
    echo "❌ GameLib not found at $GAMELIB_INSTALL_DIR"
    echo "Please build gamelib first using:"
    echo "  cd ../gamelib && ./build_standalone.sh"
    exit 1
fi

echo "✅ Found pre-built gamelib"

# Build demo
BUILD_DIR="demo_build_${DEMO_NAME}"

echo "Cleaning previous build..."
rm -rf "$BUILD_DIR"

echo "Configuring demo..."
cmake -B "$BUILD_DIR" \
    -DDEMO_NAME="$DEMO_NAME" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_PREFIX_PATH="$GAMELIB_INSTALL_DIR" \
    .

echo "Building demo..."
cmake --build "$BUILD_DIR" --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "=== Build Complete ==="
echo "Executable: $BUILD_DIR/agl_${DEMO_NAME}_demo"
echo ""
echo "To run the demo:"
echo "  ./$BUILD_DIR/agl_${DEMO_NAME}_demo"
echo ""
echo "Available demos: renderer, texture, benchmark_deltatime, example_logger"
