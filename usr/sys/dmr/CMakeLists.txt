# CMakeLists.txt for kernel C code in usr/sys/dmr
# This is primarily for analysis and attempting host compilation of C files.

# Define the source files for the dmr C code analysis library
set(DMR_C_SOURCES
    bio.c
    tty.c
    # Other .c files in this directory are mostly specific device drivers.
    # For now, we only include bio.c and tty.c as they are more general.
    # cat.c
    # dc.c
    # dh.c
    # dhdm.c
    # dhfdm.c
    # dn.c
    # dp.c
    # kl.c
    # lp.c
    # mem.c
    # partab.c # This is data, not code
    # pc.c
    # rf.c
    # rk.c
    # rp.c
    # tc.c
    # tm.c
    # vs.c
    # vt.c
)

if(DMR_C_SOURCES)
    add_library(kernel_dmr_c_code_analysis STATIC ${DMR_C_SOURCES})

    target_include_directories(kernel_dmr_c_code_analysis PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..  # This will point to usr/sys/
    )
    message(STATUS "Configuring kernel_dmr_c_code_analysis library in usr/sys/dmr/")
else()
    message(STATUS "No C sources configured for kernel_dmr_c_code_analysis in usr/sys/dmr/")
endif()
