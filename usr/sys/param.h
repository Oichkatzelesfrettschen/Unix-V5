/**
 * @file param.h
 * @brief System-wide parameters and constants for Research Unix V5.
 *
 * This file defines fundamental constants and limits for the operating system,
 * such as table sizes, process priorities, and signal numbers.
 */

/** @name System table sizes and limits */
///@{
#define	NBUF	15		/**< Number of system buffers in the buffer cache. */
#define	NINODE	100		/**< Number of in-core inodes. */
#define	NFILE	100		/**< Number of entries in the system open file table. */
#define	NMOUNT	5		/**< Maximum number of mounted file systems. */
#define	NEXEC	4		/**< Maximum number of concurrent execs (?). */
#define	MAXMEM	(32*32)		/**< Maximum core memory size in 64-byte clicks (32K words = 64K bytes). */
#define	SSIZE	20		/**< Initial stack size in 64-byte clicks for user processes. */
#define	SINCR	20		/**< Stack increment size in 64-byte clicks. */
#define	NOFILE	15		/**< Maximum number of open files per process. */
#define	CANBSIZ	256		/**< Size of canonical input buffer (for TTYs). */
#define	CMAPSIZ	100		/**< Size of the coremap (physical memory allocation map). */
#define	SMAPSIZ	100		/**< Size of the swapmap (swap space allocation map). */
#define	NCALL	20		/**< Number of entries in the callout table (for timed events). */
#define	NPROC	50		/**< Maximum number of processes. */
#define	NTEXT	20		/**< Maximum number of active shared text segments. */
#define	NCLIST	100		/**< Number of clist blocks (for TTY character buffering). */
///@}

/** @name Process priorities
 *  Lower numbers are higher priority.
 *  These values influence scheduling and swapping decisions.
 */
///@{
#define	PSWP	-100	/**< Priority for swapper process. */
#define	PINOD	-90		/**< Priority for disk I/O completion (waiting on inode). */
#define	PRIBIO	-50		/**< Priority for block I/O completion. */
#define	PPIPE	1		/**< Priority for pipe I/O. */
#define	PWAIT	40		/**< Priority for processes in `wait()`. */
#define	PSLEP	90		/**< Priority for processes sleeping for short term (e.g. tty input). */
#define	PUSER	100		/**< Base priority for user-mode processes. */
///@}

/** @name Signals
 *  Signal numbers. Do not change these values as they are part of the ABI.
 */
///@{
#define	NSIG	13		/**< Number of signals defined (1-12 are used). */
#define		SIGHUP	1	/**< Hangup. */
#define		SIGINT	2	/**< Interrupt. */
#define		SIGQIT	3	/**< Quit. */
#define		SIGINS	4	/**< Illegal Instruction. */
#define		SIGTRC	5	/**< Trace/BPT trap. */
#define		SIGIOT	6	/**< IOT instruction. */
#define		SIGEMT	7	/**< EMT instruction. */
#define		SIGFPT	8	/**< Floating Point Exception. */
#define		SIGKIL	9	/**< Kill (cannot be caught or ignored). */
#define		SIGBUS	10	/**< Bus error. */
#define		SIGSEG	11	/**< Segmentation violation. */
#define		SIGSYS	12	/**< Bad argument to system call. */
///@}

/** @name Fundamental constants
 *  These constants are deeply embedded in the system's design.
 */
///@{
#define	USIZE	16		/**< Size of U-area (per-process kernel data) in 64-byte clicks. (1KB) */
#define	NULL	0		/**< Null pointer constant. */
#define	NODEV	(-1)	/**< Non-existent device number. */
#define	ROOTINO	1		/**< Inode number of the root directory. */
#define	DIRSIZ	14		/**< Maximum length of a directory component name (excluding null terminator). */
#define BSIZE   512     /**< Block size in bytes. Fundamental unit for disk I/O. */

/** @name Device number handling */
///@{
typedef short kernel_dev_t;     /**< Kernel's internal device number type (major/minor). */
#define major(kdev) (int)(((kernel_dev_t)(kdev) >> 8) & 0xff) /**< Get major device number from kernel_dev_t. */
#define minor(kdev) (int)((kernel_dev_t)(kdev) & 0xff)       /**< Get minor device number from kernel_dev_t. */
///@}
///@}

/**
 * @struct lobyte_hibyte
 * @brief A structure to access low and high bytes of a 16-bit integer.
 * This is a common PDP-11 idiom. This specific anonymous struct definition
 * at the end of param.h seems to be a general-purpose way to think about
 * byte ordering or to provide a type for such access, though it's not directly
 * used as a named type here.
 */
struct
{
	char	lobyte;		/**< Low byte of a 16-bit value. */
	char	hibyte;		/**< High byte of a 16-bit value. */
};
