/**
 * @file slp.c
 * @brief Process sleep, wakeup, scheduling, swapping, and creation.
 *
 * Copyright 1973 Bell Telephone Laboratories Inc
 */

#include "../param.h"
#include "../user.h"   /* For u, u_error, u_procp, u_qsav, u_ssav, etc. */
#include "../proc.h"  /* For struct proc, NPROC, SRUN, SLOAD, etc. */
#include "../text.h"  /* For struct text, x_ccount, x_size, x_daddr, x_caddr */
#include "../systm.h" /* For runin, runout, runrun, coremap, swapmap, mpid */
#include "../file.h"  /* For NOFILE, struct file (used by newproc) */
#include "../inode.h" /* For struct inode (used by newproc) */
#include "../buf.h"   /* For B_READ */
#include "../seg.h"   /* For sureg related functions */

/** @brief Structure to access the PDP-11 Processor Status Word (PS). */
struct ps_word { int integ; };
/** @brief Macro to access the Processor Status word. Assumes it's at a fixed address. */
#define PS ((struct ps_word *)0177776)

// Forward declarations for functions called from this file or defined elsewhere
void wakeup(int chan);
void swtch(void);
void sched(void);
int issig(void);
void spl0(void);
void spl6(void);
void aretu(int *savearea);
int *kernel_malloc(int *map, int size_clicks);
void mfree(int *map, int size, int addr); /* From malloc.c, for coremap/swapmap */
void clearseg(int seg_addr_clicks); /* Likely assembly, clears a memory click */
int swap(int swap_addr_blocks, int core_addr_clicks, int count_clicks, int rwflag);
void xswap(struct proc *p, int write_to_swap_flag, int old_size_clicks_unused);
void panic(const char *s);
int savu(int *savearea); /* Behaves like setjmp for fork, returns 0 in child */
void retu(int uarea_base_clicks);
void idle(void);
void sureg(void);
void copyseg(int from_click_addr, int to_click_addr);
void expand(int newsize_clicks_total_image); /* Defined later in this file */
int newproc(void); /* Defined later in this file */


/**
 * @brief Put the current process to sleep on an event channel.
 * @param chan The event channel to sleep on.
 * @param pri The scheduling priority to assign while sleeping. If negative, sleep is not interruptible by signals.
 */
void sleep(int chan, int pri)
{
	register struct proc *rp;
	register int s_ps;

	u.u_dsleep = 0;
	s_ps = PS->integ;

	rp = u.u_procp;
	if(pri >= 0) {
		if(issig())
			goto psig;
		rp->p_wchan = chan;
		rp->p_stat = SWAIT;
		rp->p_pri = (char)pri;
		spl0();
		if(runin != 0) {
			runin = 0;
			wakeup((int)(long)(char *)&runin); /* Pass address as int */
		}
		swtch();
		if(issig()) {
		psig:
			aretu(u.u_qsav);
			return;
		}
	} else {
		rp->p_wchan = chan;
		rp->p_stat = SSLEEP;
		rp->p_pri = (char)pri;
		spl0();
		swtch();
	}
	PS->integ = s_ps;
}

/**
 * @brief Wake up all processes sleeping on a given event channel.
 * @param chan The event channel to wake up.
 */
void wakeup(int chan)
{
	register struct proc *p;
	register int n_awakened_swapped_out, c_chan;

loop:
	c_chan = chan;
	n_awakened_swapped_out = 0;
	for(p = &proc[0]; p < &proc[NPROC]; p++)
		if(p->p_stat != 0 && p->p_wchan == c_chan) {
			if(runout!=0 && (p->p_flag&SLOAD)==0) {
				runout = 0;
				n_awakened_swapped_out++;
			}
			p->p_wchan = 0;
			p->p_stat = SRUN;
			runrun = 1;
		}
	if(n_awakened_swapped_out) {
		chan = (int)(long)(char *)&runout;
		goto loop;
	}
}

/**
 * @brief Main scheduler loop (swapper). Runs as proc[0].
 */
void sched()
{
	struct proc *p1_candidate_toswapin = NULL;
	register struct proc *rp;
	register int a_needed_core_clicks, current_max_time_out;
	int core_address_clicks;

sloop:
	runin = 1;
	sleep((int)(long)(char *)&runin, PSWP);
	goto loop;

loop:
	spl6();
	current_max_time_out = -1;
	p1_candidate_toswapin = NULL;

	for(rp = &proc[0]; rp < &proc[NPROC]; rp++) {
		if(rp->p_stat==SRUN && (rp->p_flag&SLOAD)==0 && rp->p_time > current_max_time_out) {
			p1_candidate_toswapin = rp;
			current_max_time_out = rp->p_time;
		}
	}

	if(current_max_time_out == -1) {
		runout = 1;
		sleep((int)(long)(char *)&runout, PSWP);
		goto loop;
	}

	spl0();
	rp = p1_candidate_toswapin;
	a_needed_core_clicks = rp->p_size;
	if(rp->p_textp != NULL) {
		if(rp->p_textp->x_ccount == 0)
			a_needed_core_clicks += rp->p_textp->x_size;
	}

	if((core_address_clicks = (int)(long)kernel_malloc(coremap, a_needed_core_clicks)) != 0)
		goto found_core_for_swapin;

	spl6();
	struct proc *p1_candidate_toswapout = NULL;
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++) {
		if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD && rp->p_stat == SWAIT) {
			p1_candidate_toswapout = rp;
			goto found_victim_to_swapout;
		}
	}

	if(current_max_time_out < 3)
		goto sloop;

	int oldest_loaded_time = -1;
	for(rp = &proc[0]; rp < &proc[NPROC]; rp++) {
		if((rp->p_flag&(SSYS|SLOCK|SLOAD))==SLOAD &&
		   (rp->p_stat==SRUN || rp->p_stat==SSLEEP) &&
		    rp->p_time > oldest_loaded_time) {
			p1_candidate_toswapout = rp;
			oldest_loaded_time = rp->p_time;
		}
	}

	if(oldest_loaded_time < 2 && current_max_time_out < 5)
		goto sloop;

	if (p1_candidate_toswapout == NULL)
	    goto sloop;
	rp = p1_candidate_toswapout;

found_victim_to_swapout:
	spl0();
	rp->p_flag &= ~SLOAD;
	xswap(rp, 1, 0);
	goto loop;

found_core_for_swapin:
	rp = p1_candidate_toswapin;
	if(rp->p_textp != NULL) {
		struct text *txtp = rp->p_textp;
		if(txtp->x_ccount == 0) {
			if(swap(txtp->x_daddr, core_address_clicks, txtp->x_size, B_READ))
				goto swaper;
			txtp->x_caddr = core_address_clicks;
			core_address_clicks += txtp->x_size;
		}
		txtp->x_ccount++;
	}

	if(swap(rp->p_addr, core_address_clicks, rp->p_size, B_READ))
		goto swaper;
	mfree(swapmap, (rp->p_size+7)/8, rp->p_addr);
	rp->p_addr = core_address_clicks;
	rp->p_flag |= SLOAD;
	rp->p_time = 0;
	goto loop;

swaper:
	panic("swap error");
}

/**
 * @brief High-level context switch logic.
 */
void swtch()
{
	static struct proc *p_last_candidate = NULL;
	register int i, highest_pri;
	register struct proc *rp_current_search, *rp_chosen_next;

	if(p_last_candidate == NULL)
		p_last_candidate = &proc[0];

	savu(u.u_rsav);
	retu(proc[0].p_addr);

loop:
	rp_current_search = p_last_candidate;
	rp_chosen_next = NULL;
	highest_pri = PUSER + 20;

	for(i=0; i<NPROC; i++) {
		rp_current_search++;
		if(rp_current_search >= &proc[NPROC])
			rp_current_search = &proc[0];
		if(rp_current_search->p_stat==SRUN && (rp_current_search->p_flag&SLOAD)!=0) {
			if(rp_current_search->p_pri < highest_pri) {
				rp_chosen_next = rp_current_search;
				highest_pri = rp_current_search->p_pri;
			}
		}
	}

	if(rp_chosen_next == NULL) {
		p_last_candidate = &proc[0];
		idle();
		goto loop;
	}

	p_last_candidate = rp_chosen_next;
	retu(rp_chosen_next->p_addr);
	sureg();

	if(rp_chosen_next->p_flag & SSWAP) {
		rp_chosen_next->p_flag &= ~SSWAP;
		aretu(u.u_ssav);
	}
}

/**
 * @brief Create a new process (kernel part of fork).
 * @return 0 in child, child's PID in parent. Panics on error.
 */
int newproc()
{
	int child_core_addr, parent_core_addr;
	struct proc *p_child_proc_entry, *p_parent_proc_entry;
	register struct proc *rpp_slot_finder;
	register struct file **user_ofile_p; /* This was 'register *rip' which was problematic */
	register int n_clicks_to_copy;
	struct text *parent_textp;
	struct proc *temp_rpp; /* For file and text ref count */


	for(rpp_slot_finder = &proc[0]; rpp_slot_finder < &proc[NPROC]; rpp_slot_finder++)
		if(rpp_slot_finder->p_stat == 0)
			goto found_proc_slot;
	panic("no procs");
	return -1;

found_proc_slot:
	p_child_proc_entry = rpp_slot_finder;
	p_parent_proc_entry = u.u_procp;

	p_child_proc_entry->p_stat = SRUN;
	p_child_proc_entry->p_flag = SLOAD;
	p_child_proc_entry->p_uid = p_parent_proc_entry->p_uid;
	p_child_proc_entry->p_ttyp = p_parent_proc_entry->p_ttyp;
	p_child_proc_entry->p_textp = p_parent_proc_entry->p_textp;
	p_child_proc_entry->p_pid = ++mpid;
	p_child_proc_entry->p_ppid = p_parent_proc_entry->p_pid;
	p_child_proc_entry->p_time = 0;
	p_child_proc_entry->p_pri = p_parent_proc_entry->p_pri;


	for(user_ofile_p = &u.u_ofile[0]; user_ofile_p < &u.u_ofile[NOFILE];)
		if((*user_ofile_p) != NULL) /* Check if file pointer is valid */
			(*user_ofile_p)->f_count++; /* Increment file reference count */

	if((parent_textp = p_parent_proc_entry->p_textp) != NULL) {
		parent_textp->x_count++;
		parent_textp->x_ccount++;
	}
	if(u.u_cdir) u.u_cdir->i_count++;

	n_clicks_to_copy = p_parent_proc_entry->p_size;
	parent_core_addr = p_parent_proc_entry->p_addr;
	p_child_proc_entry->p_size = n_clicks_to_copy;

	child_core_addr = (int)(long)kernel_malloc(coremap, n_clicks_to_copy);
	if(child_core_addr == 0) {
		p_parent_proc_entry->p_stat = SIDL;
		p_child_proc_entry->p_addr = parent_core_addr;
		savu(u.u_ssav);
		xswap(p_child_proc_entry, 0, 0);
		p_child_proc_entry->p_flag |= SSWAP;
		p_parent_proc_entry->p_stat = SRUN;
		u.u_procp = p_parent_proc_entry;
		return p_child_proc_entry->p_pid;
	} else {
		p_child_proc_entry->p_addr = child_core_addr;
		int k_clicks;
		for(k_clicks=0; k_clicks < n_clicks_to_copy; k_clicks++) {
			/* copyseg expects click addresses, not byte addresses */
			copyseg(parent_core_addr + k_clicks, child_core_addr + k_clicks);
		}
	}

	/* This logic relies on setjmp/longjmp or direct stack manipulation in assembly for fork.
	 * For a C-only analysis or initial port, this part is tricky.
	 * savu(u.u_rsav) would be called by parent before child setup.
	 * Child would have its u_rsav setup to return 0 from this function.
	 * Parent would return child_pid.
	 * The current u.u_procp determines which path we are in.
	 */
	 temp_rpp = u.u_procp; /* Save current u.u_procp */
	 u.u_procp = p_child_proc_entry; /* Temporarily set context to child for its u_rsav */
	 if (savu(u.u_rsav) == 0) { /* This is the child's "return" path from fork */
	    u.u_procp = temp_rpp; /* Restore parent's context for parent's return */
	    return p_child_proc_entry->p_pid; /* Parent returns PID */
	 }
	 /* Child continues here after context switch via retu in parent */
	 u.u_procp = p_child_proc_entry; /* Ensure child's context */
	 return 0; /* Child returns 0 */
}


/**
 * @brief Expand or contract the data segment of the current process.
 * @param newsize_clicks_total_image The new total size of the process image (U-area + data + stack) in clicks.
 */
void expand(int newsize_clicks_total_image)
{
	int i, old_data_size_clicks_calc;
	register struct proc *current_p;
	int old_core_addr_clicks, new_core_addr_clicks;
	int old_total_image_size_clicks;
	int new_data_size_clicks;

	current_p = u.u_procp;
	old_total_image_size_clicks = current_p->p_size;

	old_data_size_clicks_calc = old_total_image_size_clicks - USIZE - u.u_ssize;
	if (old_data_size_clicks_calc < 0) old_data_size_clicks_calc = 0; /* Sanity */

	current_p->p_size = newsize_clicks_total_image;
	new_data_size_clicks = newsize_clicks_total_image - USIZE - u.u_ssize;
	if (new_data_size_clicks < 0) new_data_size_clicks = 0; /* Sanity */

	old_core_addr_clicks = current_p->p_addr;

	if(old_total_image_size_clicks >= newsize_clicks_total_image) {
		if (old_total_image_size_clicks > newsize_clicks_total_image) {
			mfree(coremap, old_total_image_size_clicks - newsize_clicks_total_image,
				old_core_addr_clicks + newsize_clicks_total_image);
		}
		u.u_dsize = new_data_size_clicks;
		return;
	}

	savu(u.u_rsav);
	new_core_addr_clicks = (int)(long)kernel_malloc(coremap, newsize_clicks_total_image);

	if(new_core_addr_clicks == 0) {
		savu(u.u_ssav);
		xswap(current_p, 1, old_total_image_size_clicks);
		current_p->p_flag |= SSWAP;
		swtch();
		/* After swtch, execution resumes via aretu(u.u_ssav) if swapped back in.
		   This means the code here might not be reached if swapped out and in.
		   The new_core_addr_clicks would need to be re-evaluated or passed.
		   For simplicity, this path might lead to issues in a C-only simulation without full context save/restore.
		*/
		panic("expand: swtch after failed malloc - state needs careful handling");
		return;
	}

	current_p->p_addr = new_core_addr_clicks;
	for(i=0; i < old_total_image_size_clicks; i++)
		copyseg(old_core_addr_clicks+i, new_core_addr_clicks+i);

	mfree(coremap, old_total_image_size_clicks, old_core_addr_clicks);

	int data_portion_start_click = USIZE;
	int old_data_portion_end_click = data_portion_start_click + old_data_size_clicks_calc;
	int new_data_portion_end_click = data_portion_start_click + new_data_size_clicks;

	/* Clear only the newly added part of the data segment */
	for (i = 0; i < (new_data_portion_end_click - old_data_portion_end_click); i++) {
	    clearseg(new_core_addr_clicks + old_data_portion_end_click + i);
	}

	u.u_dsize = new_data_size_clicks;

	retu(current_p->p_addr);
	sureg();
}
