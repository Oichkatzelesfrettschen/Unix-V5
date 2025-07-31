#!/bin/bash

set -e

echo "=== Unix V5 CMake Build System Test ==="

# Check prerequisites
echo "Checking prerequisites..."
for tool in cmake ninja clang gcc; do
    if command -v "$tool" &> /dev/null; then
        echo "✓ $tool found"
    else
        echo "✗ $tool not found"
        exit 1
    fi
done

echo
echo "=== Testing CMake + Ninja Build ==="

# Clean up previous builds
rm -rf build-cmake
mkdir -p build-cmake
cd build-cmake

# Configure with CMake (Debug mode with coverage and analysis)
echo "Configuring with CMake..."
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON \
    -DENABLE_ANALYSIS=ON \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    ..

echo
echo "Build configuration summary:"
echo "  Generator: Ninja"
echo "  Build type: Debug"
echo "  C Compiler: $(cmake --system-information | grep CMAKE_C_COMPILER | head -1)"
echo "  Coverage: Enabled"
echo "  Analysis: Enabled"

echo
echo "Building with Ninja (first 50 targets)..."
# Try to build just a few targets to see what works
ninja -j$(nproc) -k 0 2>&1 | head -100

echo
echo "=== Build Targets Summary ==="
echo "Available targets:"
ninja -t targets | head -20

echo
echo "=== Build System Test Complete ==="
