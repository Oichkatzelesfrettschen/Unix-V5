# Unix V5 S1 Programs Build Configuration

# List of C programs in s1
set(S1_C_PROGRAMS
    ac cal cat cc cdb check chmod cmp comm cp date dd df diff1 
    dump echo exit fc find glob goto if init kill ln login lpr ls
)

# List of Assembly programs in s1
set(S1_ASM_PROGRAMS
    ar chown clri db1 db2 db3 db4 dc1 dc2 dc3 dc4 dc5 dsw du 
    ed1 ed2 ed3 fed1 fed2 fed3 getty grep ld1 ld2 ldx lpd
    as11 as12 as13 as14 as15 as16 as17 as18 as19 
    as21 as22 as23 as24 as25 as26 as27 as28 as29
    bas0 bas1 bas2 bas3 bas4 bas5 basx
    form1 form2 form3 form4 form5 form6
)

# Multi-file programs that need special handling
set(MULTI_FILE_PROGRAMS
    diff    # diff1.c diff2.s
    db      # db1.s db2.s db3.s db4.s  
    dc      # dc1.s dc2.s dc3.s dc4.s dc5.s
    ed      # ed1.s ed2.s ed3.s
    fed     # fed1.s fed2.s fed3.s
    form    # form1.s form2.s form3.s form4.s form5.s form6.s
    ld      # ld1.s ld2.s ldx.s
    bas     # bas0.s bas1.s bas2.s bas3.s bas4.s bas5.s basx.s
)

# Add individual C programs
foreach(PROGRAM ${S1_C_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.c")
        add_unix_program(${PROGRAM})
        if(TARGET ${PROGRAM})
            add_dependencies(userland ${PROGRAM})
        endif()
    endif()
endforeach()

# Add individual Assembly programs
foreach(PROGRAM ${S1_ASM_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.s")
        add_unix_program(${PROGRAM} ASSEMBLY_ONLY)
        if(TARGET ${PROGRAM})
            add_dependencies(userland ${PROGRAM})
        endif()
    endif()
endforeach()

# Handle multi-file programs
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/diff1.c" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/diff2.s")
    add_executable(diff diff1.c diff2.s)
    target_compile_options(diff PRIVATE -Wno-implicit-function-declaration -Wno-int-conversion)
    install(TARGETS diff RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland diff)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/db1.s")
    add_executable(db db1.s db2.s db3.s db4.s)
    set_target_properties(db PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS db RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland db)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dc1.s")
    add_executable(dc dc1.s dc2.s dc3.s dc4.s dc5.s)
    set_target_properties(dc PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS dc RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland dc)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ed1.s")
    add_executable(ed ed1.s ed2.s ed3.s)
    set_target_properties(ed PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS ed RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland ed)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fed1.s")
    add_executable(fed fed1.s fed2.s fed3.s)
    set_target_properties(fed PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS fed RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland fed)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/form1.s")
    add_executable(form form1.s form2.s form3.s form4.s form5.s form6.s)
    set_target_properties(form PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS form RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland form)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ld1.s")
    add_executable(ld ld1.s ld2.s ldx.s)
    set_target_properties(ld PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS ld RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland ld)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bas0.s")
    add_executable(bas bas0.s bas1.s bas2.s bas3.s bas4.s bas5.s basx.s)
    set_target_properties(bas PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS bas RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland bas)
endif()

# Assembler components (these might be used to build a complete assembler)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/as11.s")
    add_executable(as 
        as11.s as12.s as13.s as14.s as15.s as16.s as17.s as18.s as19.s
        as21.s as22.s as23.s as24.s as25.s as26.s as27.s as28.s as29.s)
    set_target_properties(as PROPERTIES LINKER_LANGUAGE ASM)
    install(TARGETS as RUNTIME DESTINATION bin COMPONENT userland)
    add_dependencies(userland as)
endif()

message(STATUS "S1 programs configuration loaded")
message(STATUS "  C programs: ${S1_C_PROGRAMS}")
message(STATUS "  Assembly programs: ${S1_ASM_PROGRAMS}")
message(STATUS "  Multi-file programs: ${MULTI_FILE_PROGRAMS}")
