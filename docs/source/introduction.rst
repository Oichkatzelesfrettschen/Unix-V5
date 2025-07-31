Introduction
============

Welcome to the modernized Unix V5 project. This effort is dedicated to taking the historically significant Unix Version 5 codebase, originally developed for the PDP-11, and making it accessible and understandable for contemporary developers and enthusiasts.

Project Goals
-------------

1.  **Code Modernization**: Refactor the original K&R C and assembly code to more modern C standards, improving readability and compatibility with current compilers like Clang.
2.  **Build System Overhaul**: Replace the original, likely script-based, build process with a robust and flexible CMake build system.
3.  **Comprehensive Documentation**: Generate in-depth documentation of the codebase, including:
    *   API documentation for functions and data structures using Doxygen.
    *   Higher-level architectural explanations and guides using Sphinx.
4.  **x86_64 Porting**: Adapt the operating system components to run on the x86_64 architecture, allowing it to be executed on modern hardware or emulators.
5.  **Educational Resource**: Serve as an educational tool for those interested in early operating system design, kernel architecture, and the evolution of Unix.

Historical Context
------------------

Research Unix Version 5 was released by Bell Labs around 1974. It was a pivotal moment in the history of operating systems, introducing many concepts that are still fundamental today. This version was notable for being written almost entirely in C, which was a significant step towards OS portability.

Navigating this Documentation
-----------------------------

This documentation is structured to guide you through the various components of the modernized Unix V5 system.

*   **User Utilities**: Descriptions and source code documentation for the command-line utilities provided by the system.
*   **Libraries**: Information on system libraries.
*   **Kernel Internals**: Detailed explanations of the kernel's architecture, subsystems (like process management, memory management, file system), and device drivers.

We hope this project provides valuable insights into one of the most influential operating systems ever created.
