# Unix V5 S7 Programs Build Configuration
# S7 contains the roff text formatting system

# Multi-file program: roff
set(ROFF_SOURCES
    roff1.s
    roff2.s  
    roff3.s
    roff4.s
    roff5.s
    roff7.s
    roff8.s
)

# Build the roff text formatter
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/roff1.s")
    add_executable(roff ${ROFF_SOURCES})
    set_target_properties(roff PROPERTIES LINKER_LANGUAGE ASM)
    target_compile_options(roff PRIVATE -Wno-implicit-function-declaration)
    
    install(TARGETS roff
        RUNTIME DESTINATION bin
        COMPONENT userland
    )
    
    add_dependencies(userland roff)
endif()

# Install suffix table as data file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/suftab.s")
    install(FILES suftab.s
        DESTINATION share/unix-v5/roff
        COMPONENT documentation
    )
endif()

message(STATUS "S7 roff formatter configuration loaded")
message(STATUS "  Main program: roff (multi-file assembly)")
