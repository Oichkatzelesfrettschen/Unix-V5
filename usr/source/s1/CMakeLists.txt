# CMakeLists.txt — Build userland utilities in usr/source/s1

# Helper macro to add a user program
macro(add_unix_program name)
    add_executable(${name} ${name}.c)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/usr/sys)
    install(TARGETS ${name} RUNTIME DESTINATION bin COMPONENT userland)
endmacro()

# 1. Add simple C-only programs
file(GLOB S1_C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
foreach(src ${S1_C_SOURCES})
    get_filename_component(prog ${src} NAME_WE)
    add_unix_program(${prog})
    add_dependencies(userland ${prog})
endforeach()

# 2. Add simple ASM-only programs
file(GLOB S1_ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.s" "${CMAKE_CURRENT_SOURCE_DIR}/*.S")
foreach(src ${S1_ASM_SOURCES})
    get_filename_component(prog ${src} NAME_WE)
    add_executable(${prog} ${prog}.s)
    set_target_properties(${prog} PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS ${prog} RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland ${prog})
endforeach()

# 3. Multi-file utilities
# Example: diff = diff1.c + diff2.s
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/diff1.c" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/diff2.s")
    add_executable(diff diff1.c diff2.s)
    target_compile_options(diff PRIVATE -Wno-implicit-function-declaration -Wno-int-conversion)
    install(TARGETS diff RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland diff)
endif()

# Additional multi-file patterns can be added here as needed.

message(STATUS "Loaded usr/source/s1 utilities configuration:")