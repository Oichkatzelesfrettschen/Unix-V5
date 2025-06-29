# Unix V5 S3 Programs Build Configuration
# S3 contains mostly mathematical and utility library functions

# List of C programs in s3
set(S3_C_PROGRAMS
    ctime
)

# List of Assembly programs in s3
set(S3_ASM_PROGRAMS
    atan atof atoi compar crypt dpadd ecvt exp fakfp fp1 fp2 fp3 fpx 
    gamma get hypot ldiv log mesg mon nlist pow put qsort rand sin sqrt ttyn
)

# Add individual C programs with s3 prefix to avoid conflicts
foreach(PROGRAM ${S3_C_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.c")
        add_unix_program(${PROGRAM} PREFIX s3)
        if(TARGET s3_${PROGRAM})
            add_dependencies(userland s3_${PROGRAM})
        endif()
    endif()
endforeach()

# Add individual Assembly programs with s3 prefix to avoid conflicts  
foreach(PROGRAM ${S3_ASM_PROGRAMS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROGRAM}.s")
        add_unix_program(${PROGRAM} PREFIX s3 ASSEMBLY_ONLY)
        if(TARGET s3_${PROGRAM})
            add_dependencies(userland s3_${PROGRAM})
        endif()
    endif()
endforeach()

# Create a library from the mathematical functions
set(S3_LIB_SOURCES
    atan.s atof.s atoi.s compar.s crypt.s ctime.c dpadd.s ecvt.s exp.s
    fakfp.s fp1.s fp2.s fp3.s fpx.s gamma.s get.s hypot.s ldiv.s log.s
    mon.s nlist.s pow.s put.s qsort.s rand.s sin.s sqrt.s ttyn.s
)

add_library(unix_s3_lib STATIC ${S3_LIB_SOURCES})
target_compile_options(unix_s3_lib PRIVATE 
    -Wno-implicit-function-declaration 
    -Wno-int-conversion)

install(TARGETS unix_s3_lib
    ARCHIVE DESTINATION lib
    COMPONENT libraries
)

message(STATUS "S3 library functions configuration loaded")
message(STATUS "  C programs: ${S3_C_PROGRAMS}")
message(STATUS "  Assembly programs: ${S3_ASM_PROGRAMS}")
