/**
 * @file systm.h
 * @brief System-wide global variables for the Unix V5 kernel.
 *
 * This file declares various global variables used throughout the kernel,
 * including system resource maps, pointers to key data structures,
 * timekeeping variables, and scheduler/swapper flags.
 */

// Forward declarations for structs defined elsewhere but used here as pointers
struct inode;
struct buf;

extern char	canonb[CANBSIZ];	/**< Buffer for canonical TTY input processing. */
extern int	coremap[CMAPSIZ];	/**< Map for allocation of main memory. @see malloc.c */
extern int	swapmap[SMAPSIZ];	/**< Map for allocation of swap space. @see malloc.c */
extern struct inode *rootdir;	/**< Pointer to the in-core inode of the root directory. */
extern int	execnt;				/**< Count of active exec system calls. */
extern int	lbolt;				/**< System time in clock ticks since boot (updated by clock interrupt). */
extern int	time[2];			/**< Current system time (seconds since epoch, [0]=high, [1]=low). */
extern int	tout[2];			/**< Time of next callout event. */

/**
 * @struct callo
 * @brief Structure for the callout table, used for scheduling functions to be called after a timeout.
 */
struct	callo
{
	int	c_time;				/**< Time (in lbolt ticks) at which function should be called. */
	int	c_arg;				/**< Argument to be passed to the function. */
	int	(*c_func)();		/**< Pointer to the function to be called. */
} callout[NCALL];				/**< The callout table itself. */

/**
 * @struct mount
 * @brief Structure for the mount table, holding information about mounted file systems.
 */
struct	mount
{
	int	m_dev;				/**< Device number of the mounted file system. */
	struct buf *m_bufp;		/**< Pointer to the buffer containing the superblock of this fs. */
	struct inode *m_inodp;	/**< Pointer to the in-core inode of the directory on which this fs is mounted. */
} mount[NMOUNT];				/**< The mount table itself. */

extern int	mpid;				/**< Last process ID assigned. */
extern char	runin;				/**< Set to request that swapper run to swap a process in. */
extern char	runout;				/**< Set to request that swapper run to swap a process out. */
extern char	runrun;				/**< Set by wakeup to indicate scheduler should run. */
extern int	maxmem;				/**< Actual maximum available core memory in 64-byte clicks. */
extern int	*lks;				/**< Pointer to the line clock status register. */
extern int	rootdev;			/**< Device number of the root file system. */
extern int	swapdev;			/**< Device number for swap space. */
extern int	swplo;				/**< Starting block number for swap space. */
extern int	nswap;				/**< Size of swap space in blocks. */
extern int	updlock;			/**< Lock flag for the update (sync) process. */
extern int	rablock;			/**< Read-ahead block number for block devices. */
