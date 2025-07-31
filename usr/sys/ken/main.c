/**
 * @file main.c
 * @brief Kernel C entry point and core initialization for Unix V5.
 *
 * Copyright 1973 Bell Telephone Laboratories Inc
 */

#include <stdio.h> /* For printf */
#include "../param.h"
#include "../user.h" /* For struct user, u, error codes, NOFILE, DIRSIZ */
#include "../systm.h" /* For global kernel variables like rootdir, time, lks, etc. */
#include "../proc.h"  /* For struct proc, NPROC, process states/flags */
#include "../text.h"  /* For struct text */
#include "../inode.h" /* For struct inode, ROOTINO, IFMT, ILOCK etc. */
#include "../file.h"  /* For struct file */
#include "../seg.h"   /* For UISA, UISD macros and ka6 */
#include "../buf.h"   /* For struct buf */

// Forward declarations for kernel functions called from main.c
// These would ideally come from their respective headers if those headers
// are made self-contained and suitable for inclusion.
int fubyte(char *addr);
void clearseg(int seg_addr_clicks);
void mfree(int *map, int size, int addr_clicks); /* From malloc.c */
void panic(const char *s);
/* int min(int a, int b); // Defined locally or in rdwri.c */
int fuword(int *addr); /* Changed param from int to int* */
struct inode *iget(kernel_dev_t dev, int ino); /* dev is kernel_dev_t */
void cinit(void);
void binit(void);
void iinit(void);
int newproc(void);
void expand(int newsize_clicks);
int copyout(const void *kaddr, void *uaddr, int nbytes);
void sched(void);
void iput(struct inode *ip);
void prele(struct inode *ip);
void sureg(void); /* Defined later in this file */
int estabur(int nt, int nd, int ns); /* Defined later in this file */
int nseg(int n); /* Defined later in this file */

// Local definition for min if not available/linked for analysis build
#ifndef kernel_min_defined
#define kernel_min_defined
/** @brief Local definition of min to satisfy compilation for analysis. */
int min(int a, int b) { return a < b ? a : b; }
#endif

/** @brief Array of known line clock status register addresses for probing. */
int	lksp[] =
{
	0177546,
	0172540,
	0
};

/** @brief Initial code for /etc/init (PDP-11 machine code).
 * This code is copied to the first user process (init) and executed.
 * It typically consists of a few instructions to set up arguments and exec /etc/init.
 */
int	icode[] =
{
	0104413,	/**< mov $14,sp (setup stack pointer for icode execution) */
	0000014,	/**< trap to call brk(0) - set up initial break value */
	0000010,	/**< Argument for brk (address of end of data/bss, usually 0 for initial). */
	0000777,	/**< trap for exec system call (#11 in V6, V5 might differ slightly) */
	0000014,	/**< Address in user space of "/etc/init" string argument for exec */
	0000000,	/**< Null pointer for argv[1] (end of argv array for exec) */
	0062457,	/* 'e' 't' forming "/etc/init" string data */
	0061564,	/* 'c' '/' */
	0064457,	/* 'i' 'n' */
	0064556,	/* 'i' 't' */
	0000164,	/* '\0' + 'i' (part of "init" string, due to word packing) */
};

/**
 * @brief Kernel main C entry point.
 * Initializes memory, devices, mounts root, creates init process, and starts scheduler.
 * @return Typically does not return; enters scheduler or init process execution. Returns int for standard C.
 */
int main()
{
	/* extern char schar; // Seems unused */
	register int i;   /* Loop counter or temporary variable. */
	register int *p_lksp;   /* Pointer for lksp array. */

	updlock = 0;

	maxmem = 0;
	/* For host analysis, direct hardware register manipulation for memory scan is stubbed. */
	/* UISA and UISD are macros to ((struct pdp11_seg_reg *)<hardware_address>) */
	/* UISA->r[0] = (*ka6) + USIZE; */ /* Original line: Map a segment starting after proc[0]'s U-area */
	/* UISD->r[0] = ((128-1)<<8) | RW; */ /* Original line: Map 128 clicks (8KB), R/W */

	// Simplified memory initialization for host analysis:
	if (MAXMEM > 0) {
	    mfree(coremap, MAXMEM, 0);
	    maxmem = MAXMEM;
	} else {
	    maxmem = 64 * 16;
	    mfree(coremap, maxmem, 0);
	}

	printf("mem = %ldK\n", (long)maxmem * 64 / 1024);
	maxmem = min(maxmem, MAXMEM);
	mfree(swapmap, nswap, swplo);

	/* Determine clock by probing hardware addresses */
	lks = (int *)0177546; /* Default line clock status register address */
	int clock_found = 0;
	for(p_lksp=lksp; *p_lksp != 0; p_lksp++) {
		/* For host analysis, fuword will likely fail or return -1 for these addresses. */
		if(1) { /* Simulate clock found for analysis path to avoid panic */
		/* if(fuword(*p_lksp) != -1) { */ /* Original line */
			lks = (int *)(long)*p_lksp;
			clock_found = 1;
			break;
		}
	}
	if (!clock_found) { /* Check if loop finished without finding based on *p_lksp being 0 */
		/* panic("no clock"); // Real system would panic */
		printf("Warning: Clock probe skipped/defaulted for host analysis. Using default lks 0%o.\n", 0177546);
	}

	/* Set up system process (proc[0]) - the scheduler */
	proc[0].p_addr = *ka6;
	proc[0].p_size = USIZE;
	proc[0].p_stat = SRUN;
	proc[0].p_flag |= (SLOAD | SSYS);
	u.u_procp = &proc[0];

	/* Set up 'known' i-nodes and initialize kernel subsystems */
	sureg();
	if (lks) *lks = 0115;

	cinit();
	binit();
	iinit();

	rootdir = iget(rootdev, ROOTINO);
	if (rootdir) rootdir->i_flag &= ~ILOCK; else panic("no rootdir");

	u.u_cdir = iget(rootdev, ROOTINO); /* rootdev is kernel_dev_t */
	if (u.u_cdir) u.u_cdir->i_flag &= ~ILOCK; else panic("no u_cdir for proc0");

	/* Create init process (PID 1) */
	if(newproc()) {
		/* Child (init process, PID 1) path */
		expand(USIZE+1);

		u.u_uisa[0] = USIZE;
		u.u_uisd[0] = ( (1-1) << 8 ) | RW;
		for(i=1; i<8; i++) { /* Zero out other segments */
			u.u_uisa[i] = 0;
			u.u_uisd[i] = 0;
		}
		sureg();
		copyout((void*)icode, (void*)0, sizeof(icode));
		return 0;
	}
	/* Parent (proc[0], scheduler) path */
	sched();
	return 1; /* Should not be reached */
}

/**
 * @brief Set user mode segmentation registers (UISA, UISD) from current process's U-area.
 */
void sureg()
{
	register int *up_isa_entry, *up_isd_entry;
	register int i;
	register int proc_base_addr_clicks;
	register int text_adj_clicks;
	struct text *textp;

	proc_base_addr_clicks = u.u_procp->p_addr;
	up_isa_entry = &u.u_uisa[0];

	for (i = 0; i < 8; i++) {
		UISA->r[i] = up_isa_entry[i] + proc_base_addr_clicks;
	}

	text_adj_clicks = 0;
	if((textp = u.u_procp->p_textp) != NULL) {
		text_adj_clicks = proc_base_addr_clicks - textp->x_caddr;
	}

	up_isd_entry = &u.u_uisd[0];
	for (i = 0; i < 8; i++) {
		UISD->r[i] = up_isd_entry[i];
		if ((UISD->r[i] & RW) == RO && textp != NULL) { /* Corrected RWU to RW */
		    int current_seg_virt_base_clicks = u.u_uisa[i];
		    if (u.u_tsize > 0 && current_seg_virt_base_clicks < u.u_tsize ) {
			    UISA->r[i] -= text_adj_clicks;
            }
		}
	}
}

/**
 * @brief Establish user memory regions (text, data, stack segments).
 * @param nt Size of text segment in 64-byte clicks.
 * @param nd Size of data segment in 64-byte clicks.
 * @param ns Size of stack segment in 64-byte clicks.
 * @return 0 on success, -1 on error (e.g., ENOMEM).
 */
int estabur(int nt, int nd, int ns)
{
	register int virt_click_addr;
	register int *p_uisa;
	register int *p_uisd;
	int current_seg_idx;

	if(nseg(nt)+nseg(nd)+nseg(ns) > 8 || nt+nd+ns+USIZE > maxmem) {
		u.u_error = ENOMEM;
		return(-1);
	}

	p_uisa = &u.u_uisa[0];
	p_uisd = &u.u_uisd[0];
	current_seg_idx = 0;

	/* Text Segment(s) */
	virt_click_addr = 0;
	while(nt > 0 && current_seg_idx < 8) {
		int current_seg_size_clicks = min(nt, 128);
		*p_uisd = ((current_seg_size_clicks-1)<<8) | RO;
		*p_uisa = virt_click_addr;
		virt_click_addr += current_seg_size_clicks;
		nt -= current_seg_size_clicks;
		p_uisa++; p_uisd++; current_seg_idx++;
	}

	/* Data Segment(s) */
	virt_click_addr = USIZE;
	while(nd > 0 && current_seg_idx < 8) {
		int current_seg_size_clicks = min(nd, 128);
		*p_uisd = ((current_seg_size_clicks-1)<<8) | RW;
		*p_uisa = virt_click_addr;
		virt_click_addr += current_seg_size_clicks;
		nd -= current_seg_size_clicks;
		p_uisa++; p_uisd++; current_seg_idx++;
	}

	/* Stack Segment(s) - fill from the top of the 8 segments downwards */
	int stack_seg_to_fill_idx = 7;
	virt_click_addr = (8*128);

	while(ns > 0 && stack_seg_to_fill_idx >= current_seg_idx) {
		int current_seg_size_clicks = min(ns, 128);
		virt_click_addr -= current_seg_size_clicks;
		u.u_uisd[stack_seg_to_fill_idx] = ((128-current_seg_size_clicks)<<8) | RW | ED;
		u.u_uisa[stack_seg_to_fill_idx] = virt_click_addr;

		ns -= current_seg_size_clicks;
		stack_seg_to_fill_idx--;
	}

	/* Zero out any remaining unused segment descriptors */
	while(current_seg_idx <= stack_seg_to_fill_idx) {
	    u.u_uisd[current_seg_idx] = 0;
	    u.u_uisa[current_seg_idx] = 0;
	    current_seg_idx++;
	}

	sureg();
	return(0);
}

/**
 * @brief Calculates the number of 128-click (8KB) segments needed for 'n' clicks.
 * @param n Size in 64-byte clicks.
 * @return Number of segments.
 */
int nseg(int n)
{
	return((n+127)>>7);
}
