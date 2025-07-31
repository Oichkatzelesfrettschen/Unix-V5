# Install script for directory: /workspaces/Unix-V5/usr/sys

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xkernel-headersx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/sys" TYPE FILE FILES
    "/workspaces/Unix-V5/usr/sys/buf.h"
    "/workspaces/Unix-V5/usr/sys/conf.h"
    "/workspaces/Unix-V5/usr/sys/file.h"
    "/workspaces/Unix-V5/usr/sys/filsys.h"
    "/workspaces/Unix-V5/usr/sys/inode.h"
    "/workspaces/Unix-V5/usr/sys/param.h"
    "/workspaces/Unix-V5/usr/sys/proc.h"
    "/workspaces/Unix-V5/usr/sys/reg.h"
    "/workspaces/Unix-V5/usr/sys/seg.h"
    "/workspaces/Unix-V5/usr/sys/systm.h"
    "/workspaces/Unix-V5/usr/sys/text.h"
    "/workspaces/Unix-V5/usr/sys/tty.h"
    "/workspaces/Unix-V5/usr/sys/user.h"
    )
endif()

