#!/bin/bash

# Build script for the game engine project

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory
cd build

# Configure the project with CMake
echo "Configuring project with CMake..."
cmake ..

# Build the project
echo "Building project..."
cmake --build . --config Debug

echo "Build complete! Executable should be in build/bin/"
