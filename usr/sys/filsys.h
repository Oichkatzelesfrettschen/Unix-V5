/**
 * @file filsys.h
 * @brief Definition of the file system superblock structure.
 */

/**
 * @struct filsys
 * @brief Structure of the super-block read from a mounted file system.
 *
 * This structure holds metadata for a mounted file system, including its size,
 * the size of its inode list, and caches for free blocks and free inodes.
 * An in-core copy of this is typically held in a buffer associated with the
 * `struct mount` entry for the filesystem.
 */
struct	filsys
{
	int	s_isize;		/**< Size of inode list in blocks. (Number of blocks devoted to inodes from start of FS). */
	int	s_fsize;		/**< Total size of file system in blocks. */
	int	s_nfree;		/**< Number of addresses of free blocks currently in s_free[]. */
	int	s_free[100];	/**< Array caching addresses of free blocks. s_free[0] is the head of a chain if s_nfree is full. */
	int	s_ninode;		/**< Number of free inode numbers currently in s_inode[]. */
	int	s_inode[100];	/**< Array caching free inode numbers. */
	char	s_flock;	/**< Lock flag for free list manipulation. */
	char	s_ilock;	/**< Lock flag for inode free list manipulation. */
	char	s_fmod;		/**< Super-block modified flag (needs to be written back to disk). */
	char	s_ronly;	/**< Read-only flag; 0 if mounted read/write, 1 if mounted read-only. */
	int	s_time[2];	/**< Time of last super-block update ([0]=high, [1]=low). */
};
