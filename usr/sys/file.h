/**
 * @file file.h
 * @brief Definition of the system open file table structure and related constants.
 */

// Forward declaration
struct inode;

/**
 * @struct file
 * @brief System open file table entry.
 *
 * Each entry in the `file[NFILE]` array represents an open file in the system.
 * Multiple user file descriptors (in different processes or the same process)
 * can point to the same `struct file` entry (e.g., after a `dup` or `fork`).
 * This structure, in turn, points to an in-core inode.
 */
struct file {
	char	f_flag;		/**< Flags describing the mode of open (FREAD, FWRITE, FPIPE). @see File flags */
	char	f_count;	/**< Reference count (how many user file descriptors point to this entry). */
	struct inode *f_inode;	/**< Pointer to the in-core inode for this file. */
	char	*f_offset[2];	/**< Current read/write offset within the file ([0]=high, [1]=low for 64-bit offset). */
} file[NFILE];			/**< The system open file table. @see NFILE in param.h */

/** @name File flags (f_flag) */
///@{
#define	FREAD	01		/**< File is open for reading. */
#define	FWRITE	02		/**< File is open for writing. */
#define	FPIPE	04		/**< File is a pipe. */
///@}
