# CMakeLists.txt for utilities in usr/source/s1

# Add the 'cal' executable
add_executable(cal cal.c)

# Set target properties for 'cal'
# Compiler options are typically inherited from the parent CMakeLists.txt
# but can be overridden or appended here if needed.
# For example, if 'cal' needed a specific library:
# target_link_libraries(cal PRIVATE some_custom_library)

# If there were other C files in this directory for other utilities,
# they would be added similarly.
# add_executable(another_util another_util.c)

# Make sure this executable can be "installed" if an install target is defined
# in the top-level CMakeLists.txt
# install(TARGETS cal DESTINATION bin) # Example, if installing executables to 'bin'
