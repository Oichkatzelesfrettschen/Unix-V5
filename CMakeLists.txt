cmake_minimum_required(VERSION 3.10)
project(UnixV5_Modernized C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set default build type if none is specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler specific flags for x86_64, MMX, SSE (using Clang)
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(-target x86_64-unknown-linux-gnu)
  add_compile_options(-mmmx)
  add_compile_options(-msse)
  # Add other warning flags or architecture specific flags as needed
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# Option to control building of documentation
option(BUILD_DOCS "Build Doxygen and Sphinx documentation" ON)

# Placeholder for adding subdirectories, e.g. for utilities, libraries, kernel
add_subdirectory(usr/source/s1) # Userland utilities PoC (cal)
# add_subdirectory(usr/source/s2) # More userland utilities
# add_subdirectory(usr/source/s4) # C library source

# Kernel source directories (primarily for analysis build of C files)
add_subdirectory(usr/sys/conf)
add_subdirectory(usr/sys/ken)
add_subdirectory(usr/sys/dmr)


# --- Documentation Setup (Doxygen & Sphinx) ---
if(BUILD_DOCS)
  find_package(Doxygen REQUIRED) # Require Doxygen
  find_program(SPHINX_EXECUTABLE NAMES sphinx-build sphinx-build.py) # Find Sphinx executable

  if(DOXYGEN_FOUND AND SPHINX_EXECUTABLE)
    message(STATUS "Found Doxygen: ${DOXYGEN_EXECUTABLE}")
    message(STATUS "Found Sphinx: ${SPHINX_EXECUTABLE}")

    # Doxygen configuration
    set(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/Doxyfile) # Doxygen will run using this file
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doxygen_output) # Actual output dir
    set(DOXYGEN_XML_DIR ${DOXYGEN_OUTPUT_DIR}/xml) # For Breathe

    # Configure Doxyfile.in to Doxyfile in the build directory
    # This replaces @CMAKE_SOURCE_DIR@ and @CMAKE_BINARY_DIR@ placeholders
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    add_custom_target(doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT} # Use the configured Doxyfile
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR} # Run Doxygen from build dir
      COMMENT "Generating Doxygen XML documentation..."
      VERBATIM)

    # Sphinx target (depends on Doxygen finishing)
    # Note: SPHINX_CONFIG_DIR is where conf.py is located.
    # SPHINX_SOURCE_DIR is usually the same or contains it.
    set(SPHINX_CONFIG_DIR ${CMAKE_SOURCE_DIR}/docs)
    set(SPHINX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/docs/source) # Where .rst files are
    set(SPHINX_BUILD_DIR ${CMAKE_BINARY_DIR}/sphinx_output) # HTML output

    # Ensure Breathe path in conf.py is correct relative to CMAKE_BINARY_DIR
    # breathe_projects = { "UnixV5_Modernized": "<path_to_xml_output_relative_to_conf.py>" }
    # The path in conf.py is currently: ../build/doxygen_output/xml
    # If conf.py is at ${CMAKE_SOURCE_DIR}/docs/conf.py
    # And CMAKE_BINARY_DIR is ${CMAKE_SOURCE_DIR}/build (typical)
    # Then path from conf.py to ${CMAKE_BINARY_DIR}/doxygen_output/xml is indeed correct.

    add_custom_target(sphinx ALL
      COMMAND ${SPHINX_EXECUTABLE} -b html ${SPHINX_CONFIG_DIR} ${SPHINX_BUILD_DIR}
      # Working directory for sphinx: normally where conf.py is, or where it outputs
      WORKING_DIRECTORY ${SPHINX_CONFIG_DIR}
      COMMENT "Generating Sphinx HTML documentation..."
      VERBATIM)

    add_dependencies(sphinx doxygen) # Sphinx runs after Doxygen

    # A target to build just the docs
    add_custom_target(docs DEPENDS doxygen sphinx
      COMMENT "Building all documentation (Doxygen + Sphinx)")

  else()
    message(WARNING "Doxygen or Sphinx not found. Documentation will not be built.")
  endif()
endif()

# Basic install target (optional for now)
# install(TARGETS ??? DESTINATION bin) # Example

# Enable testing with CTest (optional for now)
# enable_testing()
# add_subdirectory(tests) # Example for tests

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMake C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Documentation Build: ${BUILD_DOCS}")
