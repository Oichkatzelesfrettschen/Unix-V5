Kernel `main.c` Module
========================

This module, typically found at `usr/sys/ken/main.c`, contains the primary C entry point for the kernel, `main()`, along with critical memory segmentation setup routines.

Overview
--------

After the low-level assembly bootstrap (`start` in `mch.s`) completes its initial hardware setup, it calls the `main()` function defined in this file. The responsibilities of `main()` include:

*   Initializing physical memory management (`coremap`) and swap space management (`swapmap`).
*   Detecting and initializing the system clock.
*   Setting up the process context for `proc[0]` (the initial system/scheduler process).
*   Initializing core kernel subsystems like the buffer cache (`binit`), inode table (`iinit`), and character list buffers (`cinit`).
*   Mounting the root filesystem and setting up the initial current working directory for `proc[0]`.
*   Creating the first user process (PID 1, the `init` process) using `newproc()`.
    *   The newly created `init` process has its memory space prepared and initial code (`icode`) copied to it, then it's made ready to run in user mode.
*   The original `proc[0]` then calls `sched()` to start the process scheduler.

Key Functions from `main.c`
---------------------------
The primary functions and concepts from `usr/sys/ken/main.c` include:

*   `main()`: The C entry point of the kernel.
*   `sureg()`: Sets up the PDP-11 MMU's segmentation registers from the U-area.
*   `estabur()`: Calculates and establishes segment register values in the U-area for text, data, and stack.
*   `nseg()`: Utility to calculate the number of 8KB segments needed.
*   `icode[]`: Initial machine code for the `init` process.

Details for these functions are parsed by Doxygen and available through search in the generated documentation.

Detailed Functionality
----------------------

**`main()`**:
   The sequence of operations in `main()` is crucial for bringing the system to a state where processes can run. It transitions from a bare hardware environment to a minimally functional multitasking kernel.

**`sureg()`**:
   This function is responsible for loading the User Instruction Space Address (UISA) and User Instruction Space Descriptor (UISD) registers of the PDP-11 MMU. It takes the segment definitions stored in the current process's U-area (`u.u_uisa`, `u.u_uisd`) and programs the hardware registers, effectively defining the memory map for the current process when it runs in user mode. It also handles adjustments for shared text segments.

**`estabur(nt, nd, ns)`**:
   "Establish User Regions". This function calculates the appropriate UISA/UISD register values for a process given the requested sizes for its text (`nt`), data (`nd`), and stack (`ns`) segments. It translates these sizes (in 64-byte clicks) into segment register settings, respecting the PDP-11's segmentation scheme (maximum segment size, extend-down for stacks). It updates the U-area fields which `sureg()` will later use.

**`nseg(n)`**:
   A utility function that calculates how many 128-click (8KB, which is 4K words) segments are needed for a given size `n` (in clicks).

Dependencies
------------
This module relies heavily on:
*   PDP-11 specific memory management hardware (segmentation registers).
*   Data structures defined in `param.h`, `user.h`, `proc.h`, `systm.h`, `seg.h`.
*   Low-level assembly routines for tasks like `clearseg` and `copyout`.
*   Other kernel modules for initialization (e.g., `iget` for root directory, `newproc` from `slp.c`).
