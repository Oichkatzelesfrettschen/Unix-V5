#!/bin/bash

echo "=== Unix V5 Build System - Final Demo ==="
echo

# Show system info
echo "🖥️  Development Environment:"
echo "   OS: $(lsb_release -d | cut -f2)"
echo "   CPUs: $(nproc)"
echo "   CMake: $(cmake --version | head -1)"
echo "   Ninja: $(ninja --version)"
echo "   Clang: $(clang --version | head -1)"
echo "   Cppcheck: $(cppcheck --version)"
echo

# Show build configuration
cd /workspaces/Unix-V5/build-cmake
echo "🔧 Build Configuration:"
echo "   Build Type: $(grep CMAKE_BUILD_TYPE CMakeCache.txt | cut -d= -f2)"
echo "   Generator: Ninja"
echo "   Coverage: Enabled"
echo "   Analysis: Enabled"
echo "   Total Targets: $(ninja -t targets | wc -l)"
echo

# Show key capabilities
echo "🎯 Build System Capabilities:"
echo "   ✅ Modern CMake configuration"
echo "   ✅ Fast parallel builds (Ninja)"
echo "   ✅ Code coverage infrastructure"
echo "   ✅ Static analysis integration"
echo "   ✅ Cross-compilation support"
echo "   ✅ Development-friendly defaults"
echo

# Show available commands
echo "📋 Available Commands:"
echo "   ninja                    # Build all targets"
echo "   ninja cppcheck          # Static analysis"
echo "   ninja coverage          # Coverage report"
echo "   ninja distclean         # Clean everything"
echo "   ninja kernel-analysis   # Kernel-specific analysis"
echo "   ninja userland-analysis # Userland analysis"
echo

# Show sample analysis
echo "🔍 Sample Static Analysis (first few issues):"
cd /workspaces/Unix-V5
cppcheck --enable=all --force --quiet usr/sys/ken/alloc.c 2>&1 | head -5
echo

# Show project structure
echo "📁 Project Structure:"
echo "   $(find . -name "CMakeLists.txt" | wc -l) CMakeLists.txt files"
echo "   $(find usr/sys -name "*.c" | wc -l) kernel source files"
echo "   $(find usr/source -name "*.c" | wc -l) userland C files"
echo "   $(find usr/source -name "*.s" | wc -l) assembly files"
echo

echo "✅ Unix V5 Build System Setup Complete!"
echo "   Modern development tools configured for historical Unix source code"
echo "   Ready for analysis, education, and development work"
