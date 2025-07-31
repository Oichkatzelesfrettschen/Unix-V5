cmake_minimum_required(VERSION 3.20)
project(UnixV5_Modernized
    VERSION 1.0
    DESCRIPTION "Modernized Build System for Unix V5 Port"
    LANGUAGES C ASM
)

#------------------------------------------------------------------------------
# 1. Build Configuration Options
#------------------------------------------------------------------------------
option(BUILD_KERNEL       "Build the Unix V5 kernel"                ON)
option(BUILD_USERLAND     "Build Unix V5 userland programs"         ON)
option(BUILD_COMPILER     "Build the Unix V5 C compiler"            ON)
option(USE_CLANG          "Use Clang/LLVM toolchain"                ON)
option(ENABLE_COVERAGE    "Enable code coverage analysis"           OFF)
option(ENABLE_ANALYSIS    "Enable static analysis tools"           OFF)
option(ENABLE_DOCUMENTATION "Build documentation (Doxygen & Sphinx)" OFF)

if(USE_CLANG)
    find_program(CLANG_EXE clang REQUIRED)
    set(CMAKE_C_COMPILER   ${CLANG_EXE})
    set(CMAKE_ASM_COMPILER ${CLANG_EXE})
    find_program(LLD_EXE lld REQUIRED)
    set(CMAKE_LINKER       ${LLD_EXE})
endif()

#------------------------------------------------------------------------------
# 2. Compiler Flags
#------------------------------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wno-strict-aliasing -Wno-implicit-function-declaration -Wno-incompatible-pointer-types)

# Debug/Release flags
set(CMAKE_C_FLAGS_DEBUG       "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE     "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Coverage
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Analysis
if(ENABLE_ANALYSIS)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        add_custom_target(cppcheck
            COMMAND ${CPPCHECK} --enable=all --std=c99 --platform=unix32 --suppress=missingIncludeSystem --quiet ${CMAKE_SOURCE_DIR}/usr
            COMMENT "Running Cppcheck static analysis"
        )
    endif()
    find_program(LIZARD lizard)
    if(LIZARD)
        add_custom_target(complexity
            COMMAND ${LIZARD} --length 15 --CCN 10 --arguments 5 --xml ${CMAKE_SOURCE_DIR}/usr > ${CMAKE_BINARY_DIR}/complexity-report.xml
            COMMENT "Running Lizard complexity analysis"
        )
    endif()
endif()

#------------------------------------------------------------------------------
# 3. Documentation (Doxygen & Sphinx)
#------------------------------------------------------------------------------
if(ENABLE_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    find_program(SPHINX_BUILD sphinx-build REQUIRED)

    # Configure Doxygen
    set(DOXY_IN  ${CMAKE_SOURCE_DIR}/Doxyfile.in)
    set(DOXY_OUT ${CMAKE_BINARY_DIR}/Doxyfile)
    configure_file(${DOXY_IN} ${DOXY_OUT} @ONLY)

    add_custom_target(doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating Doxygen documentation"
        VERBATIM
    )

    # Sphinx
    set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs)
    set(SPHINX_BUILD_DIR  ${CMAKE_BINARY_DIR}/docs/html)

    add_custom_target(sphinx ALL
        COMMAND ${SPHINX_BUILD} -b html ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD_DIR}
        WORKING_DIRECTORY ${SPHINX_SOURCE_DIR}
        COMMENT "Building Sphinx HTML documentation"
        VERBATIM
    )

    add_dependencies(sphinx doxygen)
    add_custom_target(docs DEPENDS doxygen sphinx COMMENT "Complete documentation build")
endif()

#------------------------------------------------------------------------------
# 4. Subdirectories
#------------------------------------------------------------------------------
if(BUILD_KERNEL)
    add_subdirectory(usr/sys)
endif()
if(BUILD_USERLAND)
    add_subdirectory(usr/source)
endif()
if(BUILD_COMPILER)
    add_subdirectory(usr/c)
endif()

#-------------------------------------------------------------------------------
# 5. Utilities
#---
# Distclean target
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Removing all generated files"
)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Summary
message(STATUS "Configuration:")
message(STATUS "  Build Type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Use Clang        : ${USE_CLANG}")
message(STATUS "  Coverage Enabled : ${ENABLE_COVERAGE}")
message(STATUS "  Analysis Enabled : ${ENABLE_ANALYSIS}")
message(STATUS "  Documentation    : ${ENABLE_DOCUMENTATION}")