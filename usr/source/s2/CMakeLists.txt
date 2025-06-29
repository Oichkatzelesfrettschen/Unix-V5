# Unix V5 S2 Programs Build Configuration

# List of C programs in s2
set(S2_C_PROGRAMS
    mail mkfs mknod mount mv nice nm nohup od pr prof ps pwd restor 
    rm sa sh size sleep sort split stty su sync tee tr typo umount uniq wc who
)

# List of Assembly programs in s2
set(S2_ASM_PROGRAMS
    mesg mkdir msh passwd pfe rew rmdir strip sum time tp1 tp2 tp3 tp4 
    tty update write
)

# Multi-file programs
set(MULTI_FILE_PROGRAMS
    tp      # tp1.s tp2.s tp3.s tp4.s
)

# Add individual C programs
foreach(PROGRAM ${S2_C_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.c")
        add_unix_program(${PROGRAM})
        if(TARGET ${PROGRAM})
            add_dependencies(userland ${PROGRAM})
        endif()
    endif()
endforeach()

# Add individual Assembly programs  
foreach(PROGRAM ${S2_ASM_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.s")
        add_unix_program(${PROGRAM} ASSEMBLY_ONLY)
        if(TARGET ${PROGRAM})
            add_dependencies(userland ${PROGRAM})
        endif()
    endif()
endforeach()

# Handle multi-file programs
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tp1.s")
    add_executable(tp tp1.s tp2.s tp3.s tp4.s)
    set_target_properties(tp PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS tp RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland tp)
endif()

message(STATUS "S2 programs configuration loaded")
message(STATUS "  C programs: ${S2_C_PROGRAMS}")
message(STATUS "  Assembly programs: ${S2_ASM_PROGRAMS}")
