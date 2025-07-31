/**
 * @file proc.h
 * @brief Definition of the process structure and related constants.
 */

// Forward declaration
struct text;

/**
 * @struct proc
 * @brief Process table entry structure.
 *
 * Each entry in the `proc` array represents a process in the system.
 * It holds essential information for managing and scheduling the process.
 */
struct proc {
	char	p_stat;		/**< Process status (see stat codes below). */
	char	p_flag;		/**< Process flags (see flag codes below). */
	char	p_pri;		/**< Process priority; lower is higher. Used for scheduling. */
	char	p_sig;		/**< Lowest numbered signal pending for this process. */
	char	p_uid;		/**< User ID of the process. */
	char	p_time;		/**< Resident time for scheduling (aging). */
	int	p_ttyp;		/**< Controlling TTY pointer (e.g., to an entry in a TTY structure array) or device number. */
	int	p_pid;		/**< Process ID. */
	int	p_ppid;		/**< Parent process ID. */
	int	p_addr;		/**< Core address of U-area if in core (clicks); disk address if swapped (blocks). */
	int	p_size;		/**< Size of process image (U-area + data + stack) in 64-byte clicks. */
	int	p_wchan;	/**< Event address on which process is sleeping (or 0 if not sleeping). */
	struct text *p_textp;	/**< Pointer to shared text segment structure (if any). */
} proc[NPROC];			/**< The process table array. @see NPROC in param.h */

/** @name Process status codes (p_stat) */
///@{
#define	SSLEEP	1		/**< Sleeping on an event usually interruptible by signals. */
#define	SWAIT	2		/**< Sleeping on an event not usually interruptible (e.g., disk I/O). */
#define	SRUN	3		/**< Runnable. */
#define	SIDL	4		/**< Intermediate state in process creation. */
#define	SZOMB	5		/**< Terminated but not yet waited for by parent (zombie). */
///@}

/** @name Process flag codes (p_flag) */
///@{
#define	SLOAD	01		/**< Process is loaded in core memory. */
#define	SSYS	02		/**< System process (e.g., swapper, init); cannot be killed by normal signals. */
#define	SLOCK	04		/**< Process is locked in core (cannot be swapped out). */
#define	SSWAP	010		/**< Process is currently being swapped out (image is not valid in core). */
///@}
