# Unix V5 Userland Programs Build Configuration

# Function to add a Unix V5 program with optional prefix for uniqueness
function(add_unix_program PROGRAM_NAME)
    set(options ASSEMBLY_ONLY)
    set(oneValueArgs SOURCE_DIR PREFIX)
    set(multiValueArgs SOURCES DEPENDENCIES)
    cmake_parse_arguments(PROG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT PROG_SOURCE_DIR)
        set(PROG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    
    # Create unique target name if prefix is provided
    if(PROG_PREFIX)
        set(TARGET_NAME "${PROG_PREFIX}_${PROGRAM_NAME}")
    else()
        set(TARGET_NAME "${PROGRAM_NAME}")
    endif()
    
    if(PROG_SOURCES)
        set(SOURCES ${PROG_SOURCES})
    else()
        # Try to find source file automatically
        if(EXISTS "${PROG_SOURCE_DIR}/${PROGRAM_NAME}.c")
            set(SOURCES ${PROGRAM_NAME}.c)
        elseif(EXISTS "${PROG_SOURCE_DIR}/${PROGRAM_NAME}.s")
            set(SOURCES ${PROGRAM_NAME}.s)
        else()
            message(WARNING "No source found for program ${PROGRAM_NAME}")
            return()
        endif()
    endif()
    
    # Check if target already exists
    if(TARGET ${TARGET_NAME})
        message(STATUS "Target ${TARGET_NAME} already exists, skipping")
        return()
    endif()
    
    # Create executable
    add_executable(${TARGET_NAME} ${SOURCES})
    
    # Set output name to original program name
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${PROGRAM_NAME})
    
    # Set Unix V5 specific compile options
    target_compile_options(${TARGET_NAME} PRIVATE
        -Wno-implicit-function-declaration
        -Wno-int-conversion
        -Wno-incompatible-pointer-types
        -Wno-main
    )
    
    if(PROG_ASSEMBLY_ONLY)
        set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE ASM)
    endif()
    
    # Add dependencies
    if(PROG_DEPENDENCIES)
        target_link_libraries(${TARGET_NAME} ${PROG_DEPENDENCIES})
    endif()
    
    # Install to bin directory
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin
        COMPONENT userland
    )
endfunction()

# Create a target that builds all userland programs BEFORE subdirectories
add_custom_target(userland
    COMMENT "Building all Unix V5 userland programs"
)

# Add subdirectories for each source collection
add_subdirectory(s1)
add_subdirectory(s2)
add_subdirectory(s3)
add_subdirectory(s4)
add_subdirectory(s7)

# Analysis target for userland
if(ENABLE_ANALYSIS)
    add_custom_target(userland-analysis
        COMMAND cppcheck --enable=all --platform=unix32 --std=c99 ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running static analysis on userland programs"
    )
    
    add_dependencies(cppcheck userland-analysis)
endif()

# Create tags for userland sources
find_program(CTAGS_EXECUTABLE ctags)
if(CTAGS_EXECUTABLE)
    add_custom_target(userland-tags
        COMMAND ${CTAGS_EXECUTABLE} -R --c-kinds=+p --fields=+S ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating tags for userland sources"
    )
endif()

message(STATUS "Userland build configuration loaded")
message(STATUS "  Source directories: s1, s2, s3, s4, s7")
