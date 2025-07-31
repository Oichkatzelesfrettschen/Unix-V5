/**
 * @file user.h
 * @brief Definition of the per-process U-area (user structure).
 *
 * The user structure (`struct user` or `u`) contains all per-process data
 * that needs to be accessible when the process is running in kernel mode.
 * It is swapped in and out with the process.
 * Its base address in kernel virtual memory for the current process is often
 * pointed to by a dedicated register or a fixed kernel address (e.g., KISA6 on PDP-11,
 * which maps to address 0140000).
 */

// Forward declaration
struct proc;
struct inode;

/**
 * @struct user
 * @brief Per-process data area (U-area).
 *
 * This structure holds information specific to a running process, such as
 * saved registers for system calls, user/group IDs, file descriptors,
 * current directory, signal handlers, accounting information, and memory
 * segmentation details.
 *
 * The comment "u = 140000" indicates its typical fixed kernel virtual address on PDP-11.
 */
struct user {
	int	u_rsav[2];		/**< Saved R5 (frame ptr) and SP (stack ptr) for `setjmp`/`longjmp` (u.u_qsav). Must be first. */
	int	u_fsav[25];		/**< Saved floating point registers (if FPP present). Must be second. */
	char	u_segflg;		/**< Flag for I/O to user or kernel space (0=user, 1=kernel). @see iomove, cpass, passc */
	char	u_error;		/**< Error number from last failed system call. */
	char	u_uid;			/**< Effective user ID. */
	char	u_gid;			/**< Effective group ID. */
	char	u_ruid;			/**< Real user ID. */
	char	u_rgid;			/**< Real group ID. */
	struct proc *u_procp;	/**< Pointer to this process's entry in the proc table. */
	char	*u_base;		/**< Base address for current I/O operation (user space). */
	char	*u_count;		/**< Bytes remaining for current I/O operation. */
	char	*u_offset[2];	/**< Current file offset for I/O (64-bit: [0]=high, [1]=low). */
	struct inode *u_cdir;	/**< Pointer to in-core inode of current directory. */
	char	u_dbuf[DIRSIZ];	/**< Buffer for current directory component name being looked up by namei. */
	char	*u_dirp;		/**< Pathname pointer for system call arguments (e.g., for namei). */
	/** @brief Directory entry structure used by namei. */
	struct	{
		int	u_ino;			/**< Inode number of current directory entry. */
		char	u_name[DIRSIZ];	/**< Name of current directory entry. */
	} u_dent;
	struct inode *u_pdir;	/**< Pointer to inode of parent directory during namei create/delete. */
	int	u_uisa[8];		/**< Per-process User Instruction Space Address registers (PDP-11 MMU). */
	int	u_uisd[8];		/**< Per-process User Instruction Space Descriptor registers (PDP-11 MMU). */
	struct file *u_ofile[NOFILE];/**< Array of pointers to system file table entries (open files). */
	int	u_arg[5];		/**< Arguments to current system call. */
	int	u_tsize;		/**< Size of text segment in 64-byte clicks. */
	int	u_dsize;		/**< Size of data segment in 64-byte clicks. */
	int	u_ssize;		/**< Size of stack segment in 64-byte clicks. */
	int	u_qsav[2];		/**< Save area for R5, SP for non-local gotos (setjmp/longjmp for signals). Same as u_rsav. */
	int	u_ssav[2];		/**< Save area for R5, SP for swapping context. */
	int	u_signal[NSIG];	/**< Signal disposition array (0=default, 1=ignore, >1=handler address). */
	int	u_utime;		/**< User time in ticks for this process. */
	int	u_stime;		/**< System time in ticks for this process. */
	int	u_cutime[2];	/**< Sum of user times of children ([0]=high, [1]=low). */
	int	u_cstime[2];	/**< Sum of system times of children ([0]=high, [1]=low). */
	int	*u_ar0;			/**< Pointer to R0 of saved user registers on trap/interrupt stack. */
	int	u_prof[4];		/**< Profiling arguments: u_prof[0]=buffer base, u_prof[1]=buffer size, u_prof[2]=pc offset, u_prof[3]=pc scale. */
	char	u_nice;			/**< Process scheduling nice value. */
	char	u_dsleep;		/**< Counter for delaying scheduling decisions (hysteresis). */
} u;	/**< The user structure for the current process. Typically at a fixed kernel address (e.g., 0140000 on PDP-11). */

/** @name System call error codes (u.u_error values) */
///@{
#define	EFAULT	106		/**< Bad address. */
#define	EPERM	1		/**< Not super-user. */
#define	ENOENT	2		/**< No such file or directory. */
#define	ESRCH	3		/**< No such process. */
#define	EIO	5		/**< I/O error. */
#define	ENXIO	6		/**< No such device or address. */
#define	E2BIG	7		/**< Arg list too long. */
#define	ENOEXEC	8		/**< Exec format error. */
#define	EBADF	9		/**< Bad file number. */
#define	ECHILD	10		/**< No children processes. */
#define	EAGAIN	11		/**< No more processes. */
#define	ENOMEM	12		/**< Not enough core. */
#define	EACCES	13		/**< Permission denied. */
#define	ENOTBLK	15		/**< Block device required. */
#define	EBUSY	16		/**< Mount device busy. */
#define	EEXIST	17		/**< File exists. */
#define	EXDEV	18		/**< Cross-device link. */
#define	ENODEV	19		/**< No such device. */
#define	ENOTDIR	20		/**< Not a directory. */
#define	EISDIR	21		/**< Is a directory. */
#define	EINVAL	22		/**< Invalid argument. */
#define	ENFILE	23		/**< File table overflow. */
#define	EMFILE	24		/**< Too many open files. */
#define	ENOTTY	25		/**< Not a typewriter. */
#define	ETXTBSY	26		/**< Text file busy. */
#define	EFBIG	27		/**< File too large. */
#define	ENOSPC	28		/**< No space left on device. */
#define	ESPIPE	29		/**< Seek on pipe. */
#define	EROFS	30		/**< Read-only file system. */
#define	EMLINK	31		/**< Too many links. */
///@}
