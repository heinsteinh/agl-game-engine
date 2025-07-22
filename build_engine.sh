#!/bin/bash

# AGL Game Engine - Main Build Script
# This script provides different build workflows for the game engine

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

show_help() {
    echo "AGL Game Engine Build System"
    echo ""
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  lib                    Build gamelib as standalone library"
    echo "  demo [NAME]           Build a specific demo using pre-built gamelib"
    echo "  clean                 Clean all build artifacts"
    echo "  full                  Full build (library + all demos integrated)"
    echo "  test [DEMO]           Quick test build and run of a demo"
    echo ""
    echo "Demo Names: renderer, texture, benchmark_deltatime, example_logger"
    echo ""
    echo "Examples:"
    echo "  $0 lib                    # Build gamelib library"
    echo "  $0 demo renderer          # Build renderer demo with pre-built lib"
    echo "  $0 test texture           # Build and run texture demo"
    echo "  $0 full                   # Full integrated build"
    echo ""
    echo "Workflow for fast iteration:"
    echo "  1. $0 lib                 # Build library once"
    echo "  2. $0 demo renderer       # Fast demo builds using pre-built lib"
    echo "  3. $0 demo texture        # Switch demos quickly"
}

build_gamelib() {
    print_header "Building GameLib as Standalone Library"
    cd gamelib
    ./build_standalone.sh Release
    cd ..
    print_success "GameLib built successfully"
}

build_demo() {
    local demo_name=${1:-renderer}
    print_header "Building Demo: $demo_name"
    cd sandbox
    ./build_demo.sh "$demo_name" Release
    cd ..
    print_success "Demo '$demo_name' built successfully"
}

clean_all() {
    print_header "Cleaning All Build Artifacts"
    rm -rf build/
    rm -rf gamelib/gamelib_build/
    rm -rf gamelib/gamelib_install/
    rm -rf sandbox/demo_build_*/
    print_success "All build artifacts cleaned"
}

full_build() {
    print_header "Full Integrated Build"
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    print_success "Full build completed"
    echo "Executables in: build/bin/"
}

test_demo() {
    local demo_name=${1:-renderer}
    print_header "Quick Test Build and Run: $demo_name"
    
    # Check if gamelib exists
    if [ ! -f "gamelib/gamelib_install/lib/libagl_gamelib.a" ]; then
        print_warning "GameLib not found, building it first..."
        build_gamelib
    fi
    
    # Build demo
    build_demo "$demo_name"
    
    # Run demo
    print_header "Running Demo: $demo_name"
    cd sandbox
    ./demo_build_${demo_name}/agl_${demo_name}_demo &
    DEMO_PID=$!
    cd ..
    
    echo "Demo started with PID: $DEMO_PID"
    echo "Press Ctrl+C to stop the demo"
    
    # Wait for user to stop
    trap "kill $DEMO_PID 2>/dev/null || true; exit 0" INT
    wait $DEMO_PID
}

# Main script logic
COMMAND=${1:-help}

case "$COMMAND" in
    "lib"|"library"|"gamelib")
        build_gamelib
        ;;
    "demo")
        build_demo "$2"
        ;;
    "clean")
        clean_all
        ;;
    "full"|"all")
        full_build
        ;;
    "test"|"run")
        test_demo "$2"
        ;;
    "help"|"-h"|"--help"|*)
        show_help
        ;;
esac
