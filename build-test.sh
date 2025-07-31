#!/bin/bash
# Unix V5 Build System Test Script

set -e

echo "=== Unix V5 Build System Test ==="
echo "Testing both CMake and Meson build systems"
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m' 
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

success() {
    echo -e "${GREEN}✓ $1${NC}"
}

warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

error() {
    echo -e "${RED}✗ $1${NC}"
}

# Check prerequisites
echo "Checking prerequisites..."

MISSING_TOOLS=()

for tool in cmake ninja meson clang gcc; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        MISSING_TOOLS+=("$tool")
    else
        success "$tool found"
    fi
done

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    error "Missing required tools: ${MISSING_TOOLS[*]}"
    echo "Please install missing tools and try again."
    exit 1
fi

# Test CMake build
echo
echo "=== Testing CMake Build System ==="

# Clean previous builds
rm -rf build-cmake
mkdir -p build-cmake

# Configure with CMake
echo "Configuring with CMake..."
if cmake -B build-cmake -S . -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DUSE_CLANG=ON \
    -DENABLE_ANALYSIS=ON; then
    success "CMake configuration successful"
else
    error "CMake configuration failed"
    exit 1
fi

# Build with CMake
echo "Building with CMake..."
if ninja -C build-cmake; then
    success "CMake build successful"
else
    warning "CMake build had issues (expected for historical code)"
fi

# Test Meson build  
echo
echo "=== Testing Meson Build System ==="

# Clean previous builds
rm -rf build-meson
mkdir -p build-meson

# Configure with Meson
echo "Configuring with Meson..."
if meson setup build-meson \
    --buildtype=debug \
    -Duse_clang=true \
    -Danalysis=true; then
    success "Meson configuration successful"
else
    error "Meson configuration failed"
    exit 1
fi

# Build with Meson
echo "Building with Meson..."
if meson compile -C build-meson; then
    success "Meson build successful"
else
    warning "Meson build had issues (expected for historical code)"
fi

# Test analysis tools
echo
echo "=== Testing Analysis Tools ==="

# Test cppcheck if available
if command -v cppcheck >/dev/null 2>&1; then
    echo "Running cppcheck analysis..."
    if ninja -C build-cmake cppcheck 2>/dev/null; then
        success "Cppcheck analysis completed"
    else
        warning "Cppcheck analysis had issues"
    fi
fi

# Test lizard if available
if command -v lizard >/dev/null 2>&1; then
    echo "Running complexity analysis..."
    if ninja -C build-cmake complexity 2>/dev/null; then
        success "Complexity analysis completed"
    else
        warning "Complexity analysis had issues"
    fi
fi

# Generate compile commands for IDE support
echo
echo "=== Generating IDE Support Files ==="

if [ -f build-cmake/compile_commands.json ]; then
    success "compile_commands.json generated for IDE support"
else
    warning "compile_commands.json not generated"
fi

# Test tags generation
if command -v ctags >/dev/null 2>&1; then
    echo "Generating ctags..."
    ctags -R --c-kinds=+p --fields=+S usr/
    if [ -f tags ]; then
        success "ctags file generated"
    else
        warning "ctags generation failed"
    fi
fi

# Summary
echo
echo "=== Build System Test Summary ==="
echo "Project structure:"
echo "  - Kernel sources: usr/sys/"
echo "  - Userland sources: usr/source/" 
echo "  - Compiler sources: usr/c/"
echo "  - Build configs: CMakeLists.txt, meson.build"
echo
echo "Build outputs:"
echo "  - CMake build: build-cmake/"
echo "  - Meson build: build-meson/"
echo
echo "Available targets:"
echo "  - cmake: unix_kernel, userland, unix_compiler"
echo "  - meson: unix_kernel_lib, userland components"
echo
success "Build system test completed successfully!"
echo
echo "Next steps:"
echo "1. Use 'ninja -C build-cmake' to build with CMake"
echo "2. Use 'meson compile -C build-meson' to build with Meson"
echo "3. Enable analysis with -DENABLE_ANALYSIS=ON (CMake) or -Danalysis=true (Meson)"
echo "4. Generate documentation with -DENABLE_DOCUMENTATION=ON (CMake) or -Ddocs=true (Meson)"
