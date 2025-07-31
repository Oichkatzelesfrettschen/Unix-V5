Scheduler and Process Management (`slp.c`)
===========================================

The `usr/sys/ken/slp.c` file is central to process lifecycle management in Unix V5. It contains the logic for process sleeping and waking, the main scheduler loop, context switching initiation, process creation (`fork`), and dynamic memory expansion for processes.

Overview
--------
This module implements the core multitasking capabilities of the kernel.

*   **Sleeping and Waking:** Processes can voluntarily give up the CPU to wait for an event using `sleep()`. Other parts of the kernel can signal that an event has occurred using `wakeup()`, making waiting processes runnable.
*   **Scheduler (`sched`):** This function is the heart of the swapper and scheduler. It runs as `proc[0]`. It decides which process to bring into memory (swap in) if it's currently swapped out, and which process to remove from memory (swap out) if space is needed.
*   **Context Switching (`swtch`):** This function is called when the current process can no longer run (e.g., it's sleeping, exiting, or its time slice is up in a preemptive system, though V5 is largely non-preemptive for user code). It selects the next runnable process from the `proc` table and switches execution to it. The actual register state switch is performed by assembly code in `mch.s`.
*   **Process Creation (`newproc`):** This function implements the kernel-side logic for the `fork()` system call. It creates a new process entry, duplicates the parent's memory image (or sets it up for copy-on-write if text is shared), and makes the new process runnable.
*   **Memory Expansion (`expand`):** This function handles requests to change the size of a process's data segment (used by the `sbrk()` system call).

Key Functions from `slp.c`
--------------------------
The primary functions and concepts from `usr/sys/ken/slp.c` include:

*   `sleep()`: Puts the current process to sleep on an event channel.
*   `wakeup()`: Wakes up processes sleeping on an event channel.
*   `sched()`: The main scheduler/swapper process (PID 0). Decides which process to swap in or out.
*   `swtch()`: Performs the high-level context switch to the next runnable process.
*   `newproc()`: Creates a new process (the kernel side of `fork()`).
*   `expand()`: Grows or shrinks the data segment of a process.

Details for these functions are parsed by Doxygen and available through search in the generated documentation.

Detailed Functionality
----------------------

**`sleep(chan, pri)`**
   Puts the current process to sleep waiting on `chan` with priority `pri`. The process state is set to `SWAIT` or `SSLEEP`. It then calls `swtch()` to yield the CPU. If a signal is pending and the priority `pri` allows interruption, the process might handle the signal instead of fully sleeping.

**`wakeup(chan)`**
   Iterates through the process table and makes any process sleeping on `chan` runnable (`SRUN`). It also sets the `runrun` flag to indicate that the scheduler (`sched`) should be run.

**`sched()`**
   This is the main loop for `proc[0]`.

   1. It looks for a runnable process that is currently swapped out (`p_stat==SRUN && (p_flag&SLOAD)==0`) and has been out the longest.
   2. If found, it checks if there's enough core memory using `malloc(coremap, ...)`.
   3. If no core, it tries to find a process to swap out:
      * First, it looks for an easy target: a loaded process in `SWAIT` state.
      * If none, and the incoming process is "deserving" (waited long enough), it looks for the oldest loaded process that is `SRUN` or `SSLEEP`.
   4. If a victim is found, it's swapped out using `xswap()`.
   5. If core is available (either initially or after swapping another out), the target process is swapped in using `swap()`. Shared text segments are handled. The process's `p_addr` is updated, and its `SLOAD` flag is set.
   6. It loops, or sleeps on `runin` / `runout` if no immediate action.

**`swtch()`**
   This is the high-level part of the context switcher.

   1. Saves the current process's context using `savu(u.u_rsav)`.
   2. Restores the context of `proc[0]` (the scheduler process) using `retu(proc[0].p_addr)`.
   3. The code then (as `proc[0]`) finds the highest priority, loaded, runnable process.
   4. If no user process is ready, it calls `idle()` (an assembly routine that waits for an interrupt).
   5. Once a process `rp` is chosen, its context is restored using `retu(rp->p_addr)`.
   6. `sureg()` is called to set up the MMU for the new process.
   7. Returns 1 (this return value is often to the assembly code that called it).

**`newproc()`**
   Implements the fork logic.

   1. Finds an empty slot in the `proc` table.
   2. Duplicates parent's state (UID, TTY, text pointer, open files with incremented counts, current directory).
   3. Assigns a new PID.
   4. Attempts to allocate memory for the child's data/stack.
   5. If memory is available, copies the parent's memory segments to the child's new space using `copyseg()`. The child process will eventually return 0 from this function path.
   6. If memory is not immediately available, the parent process might be put to sleep or the child is marked to be swapped in later. This part of V5 `newproc` is complex due to limited memory.
   7. The parent process returns the child's PID.

**`expand(newsize)`**
   Adjusts the size of the current process's data segment.

   1. If shrinking, frees the excess memory using `mfree(coremap, ...)`.
   2. If growing, attempts to `malloc(coremap, ...)` a new, larger segment.
   3. If successful and a new segment is allocated, copies the old data to the new segment and frees the old one.
   4. If memory allocation fails, it might swap out the process (`xswap`) and try again when it's swapped back in (`swtch` handles the return).

Dependencies
------------
*   PDP-11 assembly routines in `mch.s` for `savu`, `retu`, actual context switch, `idle`.
*   Memory management via `coremap`, `swapmap`, and `malloc`/`mfree` from `malloc.c`.
*   Swapping I/O routines `xswap` (kernel internal) and `swap` (from `bio.c`).
*   Data structures from `param.h`, `user.h`, `proc.h`, `text.h`.
*   MMU setup via `sureg()`.
*   Signal handling via `issig()`, `aretu()`.
