Kernel Overview
===============

This section provides an overview of the Research Unix Version 5 kernel components, focusing on the C-based implementation found primarily in `usr/sys/ken`.

Core Kernel Structure
---------------------

The V5 kernel, like many traditional Unix kernels, can be broadly divided into several key areas:

1.  **Low-Level Startup & Machine Interface:**
    *   Handled by assembly code (`usr/sys/conf/low.s`, `usr/sys/conf/mch.s`).
    *   Responsible for initial CPU setup, trap/interrupt vectoring, context switching primitives, and basic memory access routines.
    *   The C code relies heavily on these assembly routines for its interaction with the PDP-11 hardware.

2.  **Main C Entry & Initialization (`main.c`):**
    *   The `main()` function in `usr/sys/ken/main.c` is the primary entry point into the C portion of the kernel after the initial assembly bootstrap.
    *   It initializes core data structures, memory management (`coremap`, `swapmap`), device drivers (indirectly via calls like `binit`, `iinit`), mounts the root filesystem, and creates the first user process (`init`).
    *   Finally, it calls `sched()` to start the process scheduler.
    *   :doc:`main_module`

3.  **Process Management & Scheduling (`slp.c`, `proc.h`):**
    *   Manages the lifecycle of processes (creation, termination, sleeping, waking).
    *   The `sched()` function is the main scheduler loop, deciding which process to run or swap.
    *   `swtch()` handles high-level context switching logic.
    *   `newproc()` is the core of the `fork()` system call.
    *   :doc:`scheduler_proc`

4.  **Trap, Interrupt, and System Call Handling (`trap.c`, `sysent.c`, `sys*.c`):**
    *   `trap()` is the central C routine for handling all processor traps, hardware interrupts (after initial assembly dispatch), and system calls.
    *   System calls are dispatched via the `sysent[]` table to their respective implementation functions in `sys1.c`, `sys2.c`, `sys3.c`, and `sys4.c`.
    *   :doc:`trap_syscall`

5.  **Filesystem Implementation (`iget.c`, `nami.c`, `alloc.c`, `fio.c`, `rdwri.c`, various .h files):**
    *   Implements the Unix filesystem, including inode management, pathname translation, block allocation, and read/write operations on files.
    *   This layer interacts with the buffer cache (`bio.c`) and ultimately with block device drivers.
    *   *(Detailed documentation for these will be added in a subsequent section.)*

6.  **Device Drivers (`usr/sys/dmr/*`, `conf.c`):**
    *   Provide the interface between the kernel and hardware devices (disks, tapes, terminals, etc.).
    *   Each driver implements standard entry points (open, close, read, write, strategy, sgtty, interrupt handlers) defined in the `bdevsw` and `cdevsw` tables.
    *   *(Detailed documentation for these will be added in a subsequent section.)*

Navigating Kernel Documentation
-------------------------------

The subsequent pages in this section will delve into the Doxygen-generated API documentation for specific C modules within the kernel, starting with the main entry points and core process management logic.
