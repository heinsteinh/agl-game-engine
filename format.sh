#!/bin/bash

# Code formatting script using clang-format
# This script formats all C++ source files in the project

echo "🎨 Formatting C++ code with clang-format..."

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "❌ clang-format not found. Install with:"
    echo "   brew install llvm"
    echo "   # or"
    echo "   brew install clang-format"
    exit 1
fi

# Find and format all C++ files
echo "📁 Finding C++ files..."
files=$(find . -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | grep -v build | grep -v _deps)

if [ -z "$files" ]; then
    echo "❌ No C++ files found"
    exit 1
fi

echo "📝 Formatting files:"
for file in $files; do
    echo "  - $file"
    clang-format -i "$file"
done

echo "✅ Code formatting complete!"
echo "💡 Your .clang-format configuration:"
echo "   - Style: LLVM"
echo "   - Line length: 120 characters"
echo "   - Indent: 4 spaces"
