# Unix V5 S4 Programs Build Configuration
# S4 contains system library functions and runtime support

# List of C programs in s4
set(S4_C_PROGRAMS
    errlst getpw mon perror qsort rin
)

# List of Assembly programs in s4 (excluding crt0/mcrt0 which are handled separately)
set(S4_ASM_PROGRAMS
    abort atan atan2 atof chdir chmod chown close cos crand creat crypt
    dup ecvt execl execv exit exp ffltpr floor fltpr fmod fork fstat gamma
    getc getchr getcsw getgid getuid gtty hmul hsw kill ldfps link locv log
    ltod makdir mdate mknod mount nargs nice nlist open pipe pow printf
    prof putc putchr read reset retrn rsave sbrk seek setgid setuid signal
    sin sleep sqrt stat stime stty sum sync time times umount unlink wait write
)

# Add individual C programs with s4 prefix to avoid conflicts
foreach(PROGRAM ${S4_C_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.c")
        add_unix_program(${PROGRAM} PREFIX s4)
        if(TARGET s4_${PROGRAM})
            add_dependencies(userland s4_${PROGRAM})
        endif()
    endif()
endforeach()

# Add individual Assembly programs with s4 prefix to avoid conflicts
foreach(PROGRAM ${S4_ASM_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.s")
        add_unix_program(${PROGRAM} PREFIX s4 ASSEMBLY_ONLY)
        if(TARGET s4_${PROGRAM})
            add_dependencies(userland s4_${PROGRAM})
        endif()
    endif()
endforeach()

# Create C runtime library from S4 sources
set(S4_LIB_SOURCES
    abort.s atan.s atan2.s atof.s chdir.s chmod.s chown.s close.s cos.s
    crand.s creat.s crt0.s crypt.s dup.s ecvt.s errlst.c execl.s execv.s
    exit.s exp.s ffltpr.s floor.s fltpr.s fmod.s fork.s fstat.s gamma.s
    getc.s getchr.s getcsw.s getgid.s getpw.c getuid.s gtty.s hmul.s hsw.s
    kill.s ldfps.s link.s locv.s log.s ltod.s makdir.s mcrt0.s mdate.s
    mknod.s mon.c mount.s nargs.s nice.s nlist.s open.s perror.c pipe.s
    pow.s printf.s prof.s putc.s putchr.s qsort.c read.s reset.s retrn.s
    rin.c rsave.s sbrk.s seek.s setgid.s setuid.s signal.s sin.s sleep.s
    sqrt.s stat.s stime.s stty.s sum.s sync.s time.s times.s umount.s
    unlink.s wait.s write.s
)

add_library(unix_libc STATIC ${S4_LIB_SOURCES})
target_compile_options(unix_libc PRIVATE 
    -Wno-implicit-function-declaration 
    -Wno-int-conversion
    -Wno-incompatible-pointer-types)

# Special handling for runtime startup files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/crt0.s")
    add_library(unix_crt0 OBJECT crt0.s)
    set_target_properties(unix_crt0 PROPERTIES 
        LINKER_LANGUAGE ASM
        OUTPUT_NAME crt0
    )
    install(TARGETS unix_crt0
        ARCHIVE DESTINATION lib
        COMPONENT libraries
    )
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mcrt0.s")
    add_library(unix_mcrt0 OBJECT mcrt0.s)
    set_target_properties(unix_mcrt0 PROPERTIES 
        LINKER_LANGUAGE ASM
        OUTPUT_NAME mcrt0
    )
    install(TARGETS unix_mcrt0
        ARCHIVE DESTINATION lib
        COMPONENT libraries
    )
endif()

install(TARGETS unix_libc
    ARCHIVE DESTINATION lib
    COMPONENT libraries
)

# Handle Fortran-related files that were moved here
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sum.s")
    add_unix_program(fortran_sum SOURCES sum.s ASSEMBLY_ONLY)
    if(TARGET fortran_sum)
        add_dependencies(userland fortran_sum)
    endif()
endif()

# Create errors information file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/errors")
    install(FILES errors
        DESTINATION share/unix-v5
        COMPONENT documentation
    )
endif()

message(STATUS "S4 system library configuration loaded")
message(STATUS "  C programs: ${S4_C_PROGRAMS}")
message(STATUS "  Assembly programs: ${S4_ASM_PROGRAMS}")
message(STATUS "  Library: unix_libc created")
