Trap, Interrupt, and System Call Handling (`trap.c`, `sysent.c`)
=================================================================

The files `usr/sys/ken/trap.c` and `usr/sys/ken/sysent.c` are fundamental to the kernel's interaction with user processes and hardware events. `trap.c` provides the C-level handlers for all processor traps and interrupts, and it's the main dispatcher for system calls. `sysent.c` defines the system call table.

Overview of `trap.c`
--------------------
The `trap()` function is the primary C routine invoked by low-level assembly handlers (in `low.s` and `mch.s`) when a trap or interrupt occurs.

*   **Trap Types Handled:**
    *   Bus Error (SIGBUS)
    *   Illegal Instruction (SIGINS)
    *   BPT/Trace Trap (SIGTRC)
    *   IOT Instruction Trap (SIGIOT)
    *   Emulator Trap (SIGEMT, for instructions requiring emulation)
    *   Floating Point Exception (SIGFPT) - if FPP hardware/emulation present.
    *   Segmentation Violation (SIGSEG) - memory access error.
    *   System Call (`sys` instruction).
*   **Key Responsibilities:**
    *   Saves user context (registers are usually saved by assembly before calling `trap()`; `u.u_ar0` points to them).
    *   Identifies the cause of the trap/interrupt.
    *   For hardware/CPU traps, it typically prepares to send a signal to the offending process (e.g., SIGSEG, SIGBUS, SIGILL).
    *   Handles stack growth for segmentation violations if possible.
    *   For system calls:
        *   Extracts the system call number.
        *   Looks up the handler function and argument count in `sysent[]`.
        *   Copies system call arguments from user space into `u.u_arg[]`.
        *   Calls the appropriate system call function via `trap1()`.
        *   Handles return values and errors from the system call function, setting `u.u_error` and user R0.
    *   Manages signal delivery (`psig()`) before returning to user mode.
    *   Adjusts process priority and may trigger a context switch (`swtch()`).

Overview of `sysent.c`
----------------------
This file defines the `sysent[]` array (of `struct sysent`), which is the system call switch table.

*   Each entry in `sysent[]` corresponds to a system call number.
*   An entry (`struct sysent`) contains:

    1.  The number of arguments (in 16-bit words) the system call expects. These are copied from user space by `trap()`.
    2.  A pointer to the C function in the kernel that implements the system call.

*   This table is used by `trap()` to dispatch system calls to their appropriate handlers.
*   The definition of `struct sysent` and the `sysent[]` array are documented globally (originating from `usr/sys/ken/trap.c` or `usr/sys/ken/sysent.c` Doxygen comments) and can be found by searching the generated documentation.

Key Functions and Data Structures from `trap.c`
-----------------------------------------------

.. doxygenfile:: /app/usr/sys/ken/trap.c
   :project: UnixV5_Modernized

System Call Flow
----------------

1.  User process executes a `sys <call_number>` instruction (or `trap` instruction for syscalls).
2.  PDP-11 hardware traps to a fixed vector in low memory (defined in `low.s`).
3.  Assembly stub in `low.s` saves minimal state and jumps to a common assembly handler in `mch.s`.
4.  `mch.s` assembly code saves more registers, sets up kernel mode, switches to kernel stack, and calls the C `trap()` function, passing saved register values and trap type as arguments.
5.  `trap()` in `trap.c` identifies it as a system call (e.g., `dev == 6`).
6.  It fetches the system call number (often from `pc-2` relative to the user's program counter).
7.  It uses this number to index into `sysent[]` to find the argument count and handler function address.
8.  Arguments are copied from user space (pointed to by user SP or registers) into `u.u_arg[]`.
9.  `trap1()` calls the handler function (e.g., `read()`, `write()` located in `sys1.c` - `sys4.c`).
10. The handler executes, potentially calling other kernel subsystems (filesystem, scheduler, etc.).
11. The handler returns, placing its result in `u.u_ar0[R0]` and error status in `u.u_error`.
12. `trap()` prepares to return to user mode, setting processor status flags (e.g., carry bit for error) and ensuring signals are handled via `psig()`.
13. Control returns to assembly in `mch.s`, which restores user registers and executes an `rti` (Return from Trap/Interrupt) instruction.

Dependencies
------------
*   PDP-11 trap mechanism and register set.
*   Assembly routines in `mch.s` for context saving/restoring and `call` interface.
*   `sysent[]` table for dispatch.
*   System call implementation functions in `sys*.c`.
*   User area (`struct user`) for arguments, error codes, and register access.
*   Signal handling mechanisms (`psignal`, `issig`, `psig`).
*   Memory access functions (`fuword`, `fubyte`) for getting data from user space.
