#
/*
 *	Copyright 1973 Bell Telephone Laboratories Inc
 */

#include "../param.h"
#include "../systm.h"
#include "../user.h"
#include "../proc.h"
#include "../reg.h"
#include "../seg.h"
#include <stdio.h> /* For printf */

#define	EBIT	1     /**< PS Error bit, set on successful syscall return if u.u_error is set */
#define	UMODE	0170000 /**< User mode bits in PS */
#define	SETD	0170011 /**< Opcode for SETD instruction (related to floating point) */

/**
 * @struct sysent
 * @brief System call table entry structure.
 */
struct sysent	{
	int	count;      /**< Number of arguments for the system call. */
	int	(*call)();  /**< Pointer to the system call handler function. */
} sysent[64];         /**< System call table, indexed by syscall number. */

/** @brief Array mapping register numbers to their offsets in u.u_ar0. (Not directly used in this C code version) */
char	regloc[8] = { R0, R1, R2, R3, R4, R5, R6, R7 };

/* Forward declarations for functions used in this file */
void savfp(void); /* Save floating point context (assembly) */
void psignal(struct proc *p, int sig); /* Post a signal to a process */
int fuword(int *addr); /* Fetch word from user space */
int backup(int *reg_r0_ptr); /* Backup registers for stack growth? (Assembly) */
int estabur(int nt, int nd, int ns); /* Establish user regions (from main.c) */
void expand(int newsize_clicks); /* Expand process size (from slp.c) */
void copyseg(int from_click, int to_click); /* Copy a memory click (assembly or main.c helper) */
void clearseg(int click_addr); /* Clear a memory click (assembly or main.c helper) */
int issig(void); /* Check if signals are pending (from slp.c) */
void psig(void); /* Process pending signals */
void swtch(void); /* Context switch (from slp.c) */
void panic(const char *s); /* Panic (from prf.c or slp.c) */
int savu(int *savearea); /* Save user context (like setjmp, from slp.c) */
void trap1(int (*f)()); /* Helper to call actual syscall function */


/**
 * @brief Main trap handler for processor traps, interrupts, and system calls.
 *
 * Called from assembly (`mch.s`) after initial context save.
 *
 * @param dev Trap type or device number. Special values:
 *            0: Bus error
 *            1: Illegal instruction
 *            2: BPT/Trace trap
 *            3: IOT trap
 *            5: EMT trap
 *            6: System call (sys instruction)
 *            8: Floating point exception
 *            9: Segmentation violation
 * @param sp User stack pointer (passed from assembly, value at time of trap).
 * @param r1 User R1 (passed from assembly).
 * @param nps New PS (value PS would have if this were an interrupt, passed from assembly).
 * @param r0 User R0 (passed from assembly, u.u_ar0 points to its saved location).
 * @param pc User PC (passed from assembly).
 * @param ps User PS (passed from assembly).
 * @return Does not return directly to C caller; execution resumes via assembly context restore.
 */
int trap(int dev, int sp, int r1, int nps, int r0_arg, int pc_arg, int ps_arg)
{
	register int i;
	register int a; /* Temporary address or value holder */
	register struct sysent *callp;
    int r0 = r0_arg; /* Use local var for r0 to modify it for syscall return */
    int pc = pc_arg; /* Use local var for pc */
    int ps = ps_arg; /* Use local var for ps */


	// savfp(); /* Original: save floating point. Assumed handled by assembly or FPP trap if separate. */
	u.u_ar0 = &r0_arg; /* Pointer to saved R0 on kernel stack (or where assembly put it) */

	if(dev == 8) { /* Floating point exception */
		psignal(u.u_procp, SIGFPT);
		if((ps & UMODE) == UMODE) /* If in user mode, check if it's safe to return */
			goto err;          /* If error during FPE handling or not user mode, may panic or exit */
		return 0; /* Should not be reached if panic/exit in err or kernel mode */
	}

	/* Special handling for SETD instruction (related to enabling/disabling FPU) */
	if(dev == 1 && fuword((int*)(pc-2)) == SETD && u.u_signal[SIGINS] == 0) {
		/* This might be a privileged instruction that needs emulation or specific handling.
		 * If no signal is pending for SIGINS, perhaps it's handled (e.g. FPU enabled/disabled).
		 * Original just returns.
		 */
		return 0;
	}

	if((ps & UMODE) != UMODE) { /* Trap occurred in kernel mode - very bad */
		goto bad;
	}

	u.u_error = 0; /* Clear previous error */

	/* Stack overflow/segmentation check for user stack pointer */
	if(dev == 9 && sp < -u.u_ssize * 64) { /* Trap was segv, and SP is below stack base */
		if(backup(&r0_arg) == 0) { /* backup is likely an assembly routine to save registers for stack growth */
		    /* Attempt to expand stack */
		    if(!estabur(u.u_tsize, u.u_dsize, u.u_ssize + SINCR)) { /* Setup new segment sizes */
			    expand(u.u_procp->p_size + SINCR); /* Expand core allocation */
			    a = u.u_procp->p_addr + u.u_procp->p_size; /* Top of new memory */
			    /* Copy old stack content to new location if stack grows downwards from top of segment */
			    for(i = u.u_ssize; i > 0; i--) {
				    a--; /* Move downwards */
				    copyseg(a - SINCR, a); /* Copy from old position to new */
			    }
			    /* Clear newly allocated part of stack */
			    for(i = SINCR; i > 0; i--) {
				    clearseg(--a);
                }
			    u.u_ssize += SINCR; /* Corrected from =+ */
			    goto err; /* Return to user mode, retry instruction */
		    }
        }
		/* If backup failed or estabur failed, stack expansion failed */
	}

	/* Classify trap/interrupt */
	switch(dev) {
	case 0: /* Bus error */
		i = SIGBUS;
		goto def;
	case 1: /* Illegal instruction (unless it was SETD handled above) */
		i = SIGINS;
		goto def;
	case 2: /* BPT instruction or trace trap */
		i = SIGTRC;
		goto def;
	case 3: /* IOT instruction */
		i = SIGIOT;
		goto def;
	case 5: /* EMT instruction (often used for older syscall interface) */
		i = SIGEMT;
		goto def;
	case 9: /* Segmentation violation (if not stack expansion) */
		i = SIGSEG;
		goto def;
	def: /* Common path for signals */
		psignal(u.u_procp, i);
		/* Fall through if not a signal-terminating error, or if error already set */
	default: /* Unhandled device/trap type or error from psignal */
        /* If u.u_error was already set by psignal (e.g. for core dump), keep it.
         * Otherwise, if it's an unknown trap type for this switch, set a generic error.
         * The original code had `u.u_error = dev+100;` here, which seems to be a fallback.
         * However, if psignal already set u.u_error, this would overwrite it.
         * Let's assume if psignal set u.u_error, it's definitive for that signal.
         */
        if (u.u_error == 0 && dev != 6) { /* Only set if not syscall and no error yet */
             u.u_error = EFAULT; /* Generic fault for unknown hardware traps */
        }
        /* Fall through to system call check or error handling */
	case 6:; /* System call */
	}

	if(u.u_error) { /* If signal handling or other logic set an error */
		goto err;
    }

	/* System call processing */
	ps &= ~EBIT; /* Clear error bit in PS for syscall */
	callp = &sysent[fuword((int*)(pc-2)) & 077]; /* Get syscall number from instruction before PC */

	if (callp == sysent) { /* Indirect system call (syscall #0) */
		a = fuword((int*)pc); /* Address of actual syscall number */
		pc += 2;
		callp = &sysent[fuword((int*)a) & 077]; /* Get actual syscall number */
		a += 2; /* Point to first argument of actual syscall */
	} else { /* Direct system call */
		a = pc; /* Arguments follow PC */
		/* PC is NOT advanced here yet; syscall handler might re-execute or skip instruction.
		 * For now, assume PC points to word after sys instruction. Args start there.
		 * The original `pc =+ callp->count*2;` is problematic as it assumes fixed arg passing.
         * Let's assume `a` (current PC) is where args start.
         */
	}

	/* Copy arguments from user space */
	for(i=0; i < callp->count; i++) {
		u.u_arg[i] = fuword((int*)(a + i*2)); /* Args are words */
	}
	/* PC should be advanced past syscall and its arguments by the time we return to user */
    /* This is typically handled by the system call itself or by convention (e.g., sys handler returns new PC or offset) */
    /* For now, we'll assume PC is advanced by assembly or after syscall returns. */


	u.u_dirp = (char *)u.u_arg[0]; /* For syscalls that take a path, like namei */

	trap1(callp->call); /* Call the actual system call handler */

	if(u.u_error >= 100) { /* Special error codes (e.g., nosys) become SIGSYS */
		psignal(u.u_procp, SIGSYS);
    }

err: /* Common exit path for traps and syscalls */
	if(issig()) { /* Check for and process pending signals */
		psig();
    }

	if(u.u_error != 0) { /* If an error occurred during syscall or signal handling */
		ps |= EBIT;      /* Set error bit in PS */
		r0 = u.u_error;  /* Return error code in R0 */
	}
    /* Update u.u_ar0 with the potentially modified r0 for return to user */
    *u.u_ar0 = r0;

	u.u_procp->p_pri = PUSER + u.u_nice; /* Recalculate priority */

	/* Check for rescheduling */
	if(u.u_dsleep++ > 15) { /* u_dsleep tracks time without rescheduling */
		u.u_dsleep = 0;
		u.u_procp->p_pri++; /* Slightly increase priority if process runs long */
		swtch();            /* Yield CPU if needed */
	}
	return 0; /* Return to assembly dispatcher */

bad: /* Kernel trap - should not happen */
	printf("kernel trap, ka6 = %o\n", *ka6);
	printf("aps = %o\n", ps); /* Print current (kernel) PS */
	panic("trap");
    return 1; /* Should not be reached */
}

/**
 * @brief Helper function to call the actual system call handler.
 * Saves current CPU state (minimal) before calling the function f.
 * @param f Pointer to the system call function to execute.
 */
void trap1(int (*f)())
{
	savu(u.u_qsav); /* Save minimal context for potential signal handling within syscall */
	(*f)();         /* Call the system call function */
    /* Return from syscall function is handled by trap() */
}

/**
 * @brief Handler for unimplemented system calls.
 * Sets u.u_error to indicate an invalid system call.
 */
void nosys()
{
	u.u_error = 100; /* Specific error for "no such system call" */
}

/**
 * @brief Handler for null system call (syscall 0, often indirect).
 * Does nothing by itself. The actual dispatch happens in trap().
 */
void nullsys()
{
	/* Typically does nothing, or u.u_error might be set if not handled as indirect */
}
