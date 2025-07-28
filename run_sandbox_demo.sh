#!/bin/bash

# AGL Sandbox Demo Runner
# Usage: ./run_sandbox_demo.sh [demo_name]
# Available demos: shadow, projectile, renderer, mesh, texture, etc.

DEMO_NAME=${1:-shadow}

echo "Building and running $DEMO_NAME demo from sandbox..."
cd "$(dirname "$0")"

# Build the project
cd build
make -j8

if [ $? -eq 0 ]; then
    echo ""
    if [ "$DEMO_NAME" = "shadow" ]; then
        echo "=== Shadow Demo Controls ==="
        echo "WASD + Mouse: Camera movement"
        echo "F1: Toggle UI"
        echo "F2: Toggle wireframe"
        echo "F3: Toggle shadows"
        echo "F4: Toggle light animation"
        echo "ESC: Exit"
        echo ""
    elif [ "$DEMO_NAME" = "projectile" ]; then
        echo "=== Projectile Demo Controls ==="
        echo "WASD + Mouse: Camera movement"
        echo "Left Click: Shoot projectile"
        echo "R: Reset projectiles"
        echo "ESC: Exit"
        echo ""
    else
        echo "=== Demo Controls ==="
        echo "WASD + Mouse: Camera movement"
        echo "ESC: Exit"
        echo ""
    fi

    echo "Running agl_${DEMO_NAME}_demo..."
    ./bin/agl_${DEMO_NAME}_demo
else
    echo "Build failed!"
    exit 1
fi
