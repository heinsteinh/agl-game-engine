#!/bin/bash

# AGL Demo Launcher Script
# This script builds and runs different demos

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_demo() {
    echo -e "${BLUE}[DEMO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Available demos
DEMOS=("basic" "renderer" "texture" "input" "game")

show_help() {
    echo "AGL Demo Launcher"
    echo ""
    echo "Usage: $0 [demo_name] [options]"
    echo ""
    echo "Available demos:"
    echo "  basic     - Basic engine initialization and window"
    echo "  renderer  - Renderer system demonstration"
    echo "  texture   - Texture system showcase"
    echo "  input     - Input handling demonstration"
    echo "  game      - Complete game example"
    echo ""
    echo "Options:"
    echo "  --build-only    Build the demo but don't run it"
    echo "  --clean         Clean build directory before building"
    echo "  --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 renderer           # Build and run renderer demo"
    echo "  $0 texture --clean    # Clean build and run texture demo"
    echo "  $0 game --build-only  # Build game demo but don't run"
}

# Parse arguments
DEMO_NAME=""
BUILD_ONLY=false
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --build-only)
            BUILD_ONLY=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        -*)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
        *)
            if [ -z "$DEMO_NAME" ]; then
                DEMO_NAME="$1"
            else
                print_error "Multiple demo names specified"
                show_help
                exit 1
            fi
            shift
            ;;
    esac
done

# If no demo specified, show interactive menu
if [ -z "$DEMO_NAME" ]; then
    echo "AGL Demo Launcher - Select a demo:"
    echo ""
    for i in "${!DEMOS[@]}"; do
        echo "$((i+1)). ${DEMOS[$i]}"
    done
    echo ""
    read -p "Enter demo number (1-${#DEMOS[@]}): " choice

    if [[ "$choice" =~ ^[1-9][0-9]*$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "${#DEMOS[@]}" ]; then
        DEMO_NAME="${DEMOS[$((choice-1))]}"
    else
        print_error "Invalid selection"
        exit 1
    fi
fi

# Validate demo name
if [[ ! " ${DEMOS[@]} " =~ " ${DEMO_NAME} " ]]; then
    print_error "Unknown demo: $DEMO_NAME"
    echo "Available demos: ${DEMOS[*]}"
    exit 1
fi

print_info "Selected demo: $DEMO_NAME"

# Clean build if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and build
print_info "Configuring CMake for demo: $DEMO_NAME"
if ! cmake -DDEMO_NAME="$DEMO_NAME" ..; then
    print_error "CMake configuration failed"
    exit 1
fi

print_info "Building demo..."
if ! make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4); then
    print_error "Build failed"
    exit 1
fi

EXECUTABLE="./bin/agl_${DEMO_NAME}_demo"

if [ ! -f "$EXECUTABLE" ]; then
    print_error "Executable not found: $EXECUTABLE"
    exit 1
fi

print_info "Build successful!"

if [ "$BUILD_ONLY" = true ]; then
    print_info "Build-only mode. Executable: $EXECUTABLE"
    exit 0
fi

# Run the demo
print_demo "Starting $DEMO_NAME demo..."
print_info "Press Ctrl+C to stop the demo"
echo ""

"$EXECUTABLE"

print_demo "Demo finished."
