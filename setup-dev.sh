#!/bin/bash

# Development Setup Script for C++17 Game Engine Project
# This script helps set up the development environment

echo "Setting up C++17 Game Engine Development Environment..."

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_tip() {
    echo -e "${YELLOW}[TIP]${NC} $1"
}

# Check if git is initialized
if [ ! -d ".git" ]; then
    print_info "Initializing Git repository..."
    git init
fi

# Ensure pre-commit hook is executable
if [ -f ".git/hooks/pre-commit" ]; then
    chmod +x .git/hooks/pre-commit
    print_info "Pre-commit hook is now executable"
else
    print_tip "Pre-commit hook not found. It should be created automatically."
fi

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
    print_info "Created build directory"
fi

# Display information about the development setup
echo ""
echo "=== Development Environment Setup Complete ==="
echo ""
print_info "Project Structure:"
echo "  - Source files: src/"
echo "  - Headers: include/"
echo "  - Build output: build/"
echo "  - Dependencies managed by: Dependency.cmake"
echo ""
print_info "Git Hooks Configured:"
echo "  - Pre-commit hook: Runs formatting checks, C++ linting, and build verification"
echo ""
print_info "Available Build Commands:"
echo "  mkdir -p build && cd build && cmake .. && make"
echo ""
print_info "Pre-commit Hook Features:"
echo "  ✓ Code formatting checks (trailing whitespace, tabs, long lines)"
echo "  ✓ C++ best practices checks (include guards, smart pointers)"
echo "  ✓ TODO/FIXME detection"
echo "  ✓ Build verification"
echo ""
print_tip "To skip build check during commit (for faster commits):"
echo "  SKIP_BUILD_CHECK=1 git commit -m 'your message'"
echo ""
print_tip "Recommended VS Code extensions:"
echo "  - C/C++ (Microsoft)"
echo "  - CMake Tools"
echo "  - GitLens"
echo "  - Bracket Pair Colorizer"
echo ""
print_info "Happy coding! 🚀"
