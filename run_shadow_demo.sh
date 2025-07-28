#!/bin/bash

echo "Building and running Shadow Demo from Sandbox..."
cd "$(dirname "$0")"

# Build the project
cd build
make -j8

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Shadow Demo Controls ==="
    echo "WASD + Mouse: Camera movement"
    echo "F1: Toggle UI"
    echo "F2: Toggle wireframe"
    echo "F3: Toggle shadows"
    echo "F4: Toggle light animation"
    echo "ESC: Exit"
    echo ""
    echo "Running shadow demo from sandbox..."
    ./bin/agl_shadow_demo
else
    echo "Build failed!"
    exit 1
fi
