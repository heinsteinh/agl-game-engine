#!/bin/bash

# AGL GameLib Standalone Build Script
# This script builds the gamelib as a standalone library

set -e

echo "=== Building AGL GameLib as Standalone Library ==="

# Configuration
BUILD_TYPE=${1:-Release}
INSTALL_PREFIX=${2:-$(pwd)/gamelib_install}
BUILD_DIR="gamelib_build"

echo "Build Type: $BUILD_TYPE"
echo "Install Prefix: $INSTALL_PREFIX"
echo "Build Directory: $BUILD_DIR"
echo ""

# Clean previous build
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring..."
cmake -f ../standalone_build.cmake \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    ..

# Build
echo "Building..."
cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Install
echo "Installing..."
cmake --install .

echo ""
echo "=== Build Complete ==="
echo "Library installed to: $INSTALL_PREFIX"
echo "Library file: $INSTALL_PREFIX/lib/libagl_gamelib.a"
echo "Headers: $INSTALL_PREFIX/include/agl/"
echo "CMake package: $INSTALL_PREFIX/lib/cmake/agl-gamelib/"
echo ""
echo "To use in other projects:"
echo "  find_package(agl-gamelib REQUIRED)"
echo "  target_link_libraries(your_target AGL::gamelib)"
