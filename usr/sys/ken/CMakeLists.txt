# CMakeLists.txt for kernel C code in usr/sys/ken
# This is primarily for analysis and attempting host compilation of C files.

# Define the source files for the kernel C code analysis library
set(KERNEL_C_SOURCES
    main.c
    slp.c
    trap.c
    alloc.c
    fio.c
    iget.c
    nami.c
    pipe.c
    prf.c
    rdwri.c
    sig.c
    subr.c
    sys1.c
    sys2.c
    sys3.c
    sys4.c
    sysent.c
    text.c
    malloc.c # It's in ken/ in this layout
    # Add other .c files from usr/sys/ken as needed
)

# Add a library target for these C files
# This won't be a functional kernel component for x86_64 without significant porting
# but allows for isolated compilation for analysis and initial C-level issue spotting.
add_library(kernel_c_code_analysis STATIC ${KERNEL_C_SOURCES})

# Set include directories. The sources use includes like "../param.h" or "../user.h"
# so the include directory should be the parent of "ken", which is "usr/sys"
# CMAKE_CURRENT_SOURCE_DIR here is /app/usr/sys/ken
target_include_directories(kernel_c_code_analysis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # This will point to usr/sys/
)

# We might need to define some PDP-11 specific types or macros if they prevent compilation
# For example, if register keyword causes issues with modern clang for host analysis:
# target_compile_definitions(kernel_c_code_analysis PRIVATE -Dregister=)
# This is just an example, apply as needed based on compilation errors.

# Add Doxygen comments from these files to the overall documentation
# This is implicitly handled by Doxygen parsing the whole INPUT directory for now.
# If we wanted specific Doxygen targets per library later, we could add them.

message(STATUS "Configuring kernel_c_code_analysis library in usr/sys/ken/")
