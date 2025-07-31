# Unix V5 C Compiler Build Configuration

# C Compiler source files
set(C_COMPILER_SOURCES
    c00.c   # Main compiler driver
    c01.c   # Lexical analyzer
    c02.c   # Parser
    c03.c   # Expression parsing
    c04.c   # Code generation
    c0h.c   # Header/definitions
    cvopt.c # Optimizer
)

# Assembly sources
set(C_COMPILER_ASM_SOURCES
    c0t.s   # Code generation tables
    c1t.s   # Additional tables
    cctab.s # Compiler tables
    efftab.s # Efficiency tables
    regtab.s # Register allocation tables
    sptab.s  # Special tables
)

# Separate compilation phases
set(C0_SOURCES c00.c c01.c c02.c c03.c c04.c c0h.c c0t.s)
set(C1_SOURCES c10.c c11.c c12.c c13.c c1h.c c1t.s)
set(C2_SOURCES c20.c c21.c c2h.c)

# Build first pass of compiler (c0)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/c00.c")
    add_executable(c0 ${C0_SOURCES})
    target_compile_options(c0 PRIVATE 
        -Wno-implicit-function-declaration
        -Wno-int-conversion
        -Wno-incompatible-pointer-types
        -Wno-main
    )
    
    install(TARGETS c0
        RUNTIME DESTINATION lib/unix-v5
        COMPONENT compiler
    )
endif()

# Build second pass of compiler (c1) 
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/c10.c")
    add_executable(c1 ${C1_SOURCES})
    target_compile_options(c1 PRIVATE 
        -Wno-implicit-function-declaration
        -Wno-int-conversion
        -Wno-incompatible-pointer-types
    )
    
    install(TARGETS c1
        RUNTIME DESTINATION lib/unix-v5
        COMPONENT compiler
    )
endif()

# Build third pass of compiler (c2)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/c20.c")
    add_executable(c2 ${C2_SOURCES})
    target_compile_options(c2 PRIVATE 
        -Wno-implicit-function-declaration
        -Wno-int-conversion
        -Wno-incompatible-pointer-types
    )
    
    install(TARGETS c2
        RUNTIME DESTINATION lib/unix-v5
        COMPONENT compiler
    )
endif()

# Build optimizer
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cvopt.c")
    add_executable(cvopt cvopt.c)
    target_compile_options(cvopt PRIVATE 
        -Wno-implicit-function-declaration
        -Wno-int-conversion
    )
    
    install(TARGETS cvopt
        RUNTIME DESTINATION lib/unix-v5
        COMPONENT compiler
    )
endif()

# Install compiler tables as data files
set(COMPILER_TABLES cctab.s efftab.s regtab.s sptab.s)
foreach(TABLE ${COMPILER_TABLES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TABLE}")
        install(FILES ${TABLE}
            DESTINATION lib/unix-v5/tables
            COMPONENT compiler
        )
    endif()
endforeach()

# Create a wrapper script for the complete compiler
configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/cc.sh.in
    ${CMAKE_BINARY_DIR}/cc
    @ONLY
)

install(PROGRAMS ${CMAKE_BINARY_DIR}/cc
    DESTINATION bin
    COMPONENT compiler
)

# Create target for all compiler components
add_custom_target(unix_compiler
    DEPENDS c0 c1 c2 cvopt
    COMMENT "Building Unix V5 C compiler"
)

message(STATUS "C Compiler configuration loaded")
message(STATUS "  Compiler phases: c0, c1, c2")
message(STATUS "  Optimizer: cvopt")
message(STATUS "  Tables: ${COMPILER_TABLES}")
