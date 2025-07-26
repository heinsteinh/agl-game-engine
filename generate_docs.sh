#!/bin/bash

# AGL Game Engine - Documentation Generation Script
# Generates comprehensive camera system documentation using Doxygen

echo "=== AGL Game Engine Documentation Generator ==="
echo "Generating camera system documentation..."

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed."
    echo "Please install Doxygen:"
    echo "  macOS: brew install doxygen"
    echo "  Ubuntu: sudo apt-get install doxygen"
    echo "  Windows: Download from https://www.doxygen.nl/download.html"
    exit 1
fi

# Create docs directory if it doesn't exist
mkdir -p docs/doxygen

# Generate documentation
echo "Running Doxygen..."
doxygen Doxyfile

if [ $? -eq 0 ]; then
    echo "✅ Documentation generated successfully!"
    echo ""
    echo "Documentation files:"
    echo "  📄 Markdown Guide: docs/CAMERA_DOCUMENTATION.md"
    echo "  🌐 HTML Docs:      docs/doxygen/html/index.html"
    echo ""
    echo "To view the HTML documentation:"
    echo "  macOS:   open docs/doxygen/html/index.html"
    echo "  Linux:   xdg-open docs/doxygen/html/index.html"
    echo "  Windows: start docs/doxygen/html/index.html"
    echo ""
    echo "📚 The documentation includes:"
    echo "  • Complete API reference for Camera and CameraController classes"
    echo "  • Detailed usage examples and code snippets"
    echo "  • Configuration guides for different game types"
    echo "  • Troubleshooting section with common issues"
    echo "  • Advanced features documentation"
else
    echo "❌ Error generating documentation"
    exit 1
fi
