# Unix V5 Kernel Build Configuration

# Kernel sources from ken/ directory
set(KERNEL_KEN_SOURCES
    ken/alloc.c
    ken/clock.c
    ken/fio.c
    ken/iget.c
    ken/main.c
    ken/malloc.c
    ken/nami.c
    ken/pipe.c
    ken/prf.c
    ken/rdwri.c
    ken/sig.c
    ken/slp.c
    ken/subr.c
    ken/sys1.c
    ken/sys2.c
    ken/sys3.c
    ken/sys4.c
    ken/sysent.c
    ken/text.c
    ken/trap.c
)

# Device driver sources from dmr/ directory
set(KERNEL_DMR_SOURCES
    dmr/bio.c
    dmr/cat.c
    dmr/dc.c
    dmr/dh.c
    dmr/dhdm.c
    dmr/dhfdm.c
    dmr/dn.c
    dmr/dp.c
    dmr/kl.c
    dmr/lp.c
    dmr/mem.c
    dmr/partab.c
    dmr/pc.c
    dmr/rf.c
    dmr/rk.c
    dmr/rp.c
    dmr/tc.c
    dmr/tm.c
    dmr/tty.c
    dmr/vs.c
    dmr/vt.c
)

# Configuration sources from conf/ directory
set(KERNEL_CONF_SOURCES
    conf/conf.c
    conf/mkconf.c
)

# Assembly sources
set(KERNEL_ASM_SOURCES
    conf/low.s
    conf/mch.s
)

# Header files
set(KERNEL_HEADERS
    buf.h
    conf.h
    file.h
    filsys.h
    inode.h
    param.h
    proc.h
    reg.h
    seg.h
    systm.h
    text.h
    tty.h
    user.h
)

# Create kernel library for analysis
add_library(unix_kernel_lib STATIC
    ${KERNEL_KEN_SOURCES}
    ${KERNEL_DMR_SOURCES}
    ${KERNEL_CONF_SOURCES}
)

# Set kernel-specific compile flags
target_compile_definitions(unix_kernel_lib PRIVATE
    KERNEL=1
    UNIX_V5=1
)

target_compile_options(unix_kernel_lib PRIVATE
    -fno-builtin
    -nostdlib
    -ffreestanding
    -Wno-main
)

# Create mkconf utility for kernel configuration
add_executable(mkconf conf/mkconf.c)
target_compile_options(mkconf PRIVATE -Wno-implicit-function-declaration)

# Custom target to generate configuration
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c.c
    COMMAND mkconf > ${CMAKE_CURRENT_BINARY_DIR}/c.c
    DEPENDS mkconf
    COMMENT "Generating kernel configuration"
)

# Main kernel target (requires custom linking for actual kernel)
add_custom_target(unix_kernel
    DEPENDS unix_kernel_lib mkconf ${CMAKE_CURRENT_BINARY_DIR}/c.c
    COMMENT "Building Unix V5 kernel components"
)

# Install kernel headers for userland builds
install(FILES ${KERNEL_HEADERS}
    DESTINATION include/sys
    COMPONENT kernel-headers
)

# Analysis targets for kernel
if(ENABLE_ANALYSIS)
    add_custom_target(kernel-analysis
        COMMAND cppcheck --enable=all --platform=unix32 --std=c99 ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running static analysis on kernel"
    )
    
    add_dependencies(cppcheck kernel-analysis)
endif()

# Create tags for kernel sources
find_program(CTAGS_EXECUTABLE ctags)
if(CTAGS_EXECUTABLE)
    add_custom_target(kernel-tags
        COMMAND ${CTAGS_EXECUTABLE} -R --c-kinds=+p --fields=+S ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating tags for kernel sources"
    )
endif()

message(STATUS "Kernel build configuration loaded")
message(STATUS "  Ken sources: ${CMAKE_CURRENT_LIST_DIR}/ken/")
message(STATUS "  DMR sources: ${CMAKE_CURRENT_LIST_DIR}/dmr/")
message(STATUS "  Conf sources: ${CMAKE_CURRENT_LIST_DIR}/conf/")
