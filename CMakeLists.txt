cmake_minimum_required(VERSION 3.20)

project(UnixV5
    VERSION 1.0.0
    DESCRIPTION "Unix Version 5 Source Code Build System"
    LANGUAGES C ASM
)

# Set C standard to match original Unix V5 era
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build configuration options
option(BUILD_KERNEL "Build Unix V5 kernel" ON)
option(BUILD_USERLAND "Build Unix V5 userland programs" ON)
option(BUILD_COMPILER "Build Unix V5 C compiler" ON)
option(USE_CLANG "Use Clang/LLVM toolchain" ON)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)
option(ENABLE_ANALYSIS "Enable static analysis tools" OFF)
option(ENABLE_DOCUMENTATION "Build documentation" OFF)

# Set compiler preferences
if(USE_CLANG)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_ASM_COMPILER clang)
    set(CMAKE_LINKER lld)
endif()

# Global compiler flags for Unix V5 compatibility
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")

# Debug/Release configurations
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Enable coverage if requested
if(ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Enable analysis by default for debugging
if(NOT DEFINED ENABLE_ANALYSIS)
    set(ENABLE_ANALYSIS ON)
endif()

# Enable coverage by default for debugging
if(NOT DEFINED ENABLE_COVERAGE)
    set(ENABLE_COVERAGE ON)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Include directories
include_directories(usr/sys)

# Custom target for cleaning generated files
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all generated files"
)

# Analysis targets
if(ENABLE_ANALYSIS)
    # Cppcheck target
    find_program(CPPCHECK_EXECUTABLE cppcheck)
    if(CPPCHECK_EXECUTABLE)
        add_custom_target(cppcheck
            COMMAND ${CPPCHECK_EXECUTABLE}
                --enable=all
                --std=c99
                --platform=unix32
                --suppress=missingIncludeSystem
                --quiet
                --xml
                --xml-version=2
                --output-file=${CMAKE_BINARY_DIR}/cppcheck-report.xml
                ${CMAKE_SOURCE_DIR}/usr
            COMMENT "Running Cppcheck static analysis"
        )
    endif()

    # Lizard complexity analysis
    find_program(LIZARD_EXECUTABLE lizard)
    if(LIZARD_EXECUTABLE)
        add_custom_target(complexity
            COMMAND ${LIZARD_EXECUTABLE}
                --length 15
                --CCN 10
                --arguments 5
                --xml
                ${CMAKE_SOURCE_DIR}/usr
                > ${CMAKE_BINARY_DIR}/complexity-report.xml
            COMMENT "Running Lizard complexity analysis"
        )
    endif()
endif()

# Coverage targets
if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage-clean
            COMMAND ${LCOV_EXECUTABLE} --directory . --zerocounters
            COMMENT "Cleaning coverage data"
        )
        
        add_custom_target(coverage-capture
            COMMAND ${LCOV_EXECUTABLE} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*' '*/test/*' --output-file coverage-filtered.info
            COMMENT "Capturing coverage data"
        )
        
        add_custom_target(coverage-report
            COMMAND ${GENHTML_EXECUTABLE} coverage-filtered.info --output-directory coverage-html
            COMMENT "Generating HTML coverage report"
        )
        
        add_custom_target(coverage
            DEPENDS coverage-clean coverage-capture coverage-report
            COMMENT "Complete coverage analysis"
        )
    endif()
endif()

# Documentation target
if(ENABLE_DOCUMENTATION)
    find_program(DOXYGEN_EXECUTABLE doxygen)
    if(DOXYGEN_EXECUTABLE)
        configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating documentation with Doxygen"
        )
    endif()
endif()

# Add subdirectories
if(BUILD_KERNEL)
    add_subdirectory(usr/sys)
endif()

if(BUILD_USERLAND)
    add_subdirectory(usr/source)
endif()

if(BUILD_COMPILER)
    add_subdirectory(usr/c)
endif()

# Create compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Summary information
message(STATUS "Unix V5 Build Configuration:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Build Kernel: ${BUILD_KERNEL}")
message(STATUS "  Build Userland: ${BUILD_USERLAND}")
message(STATUS "  Build Compiler: ${BUILD_COMPILER}")
message(STATUS "  Use Clang: ${USE_CLANG}")
message(STATUS "  Enable Coverage: ${ENABLE_COVERAGE}")
message(STATUS "  Enable Analysis: ${ENABLE_ANALYSIS}")
