/**
 * @file inode.h
 * @brief Definition of the in-core inode structure and related constants.
 */

/**
 * @struct inode
 * @brief In-core representation of a file system inode.
 *
 * This structure holds information about a file, such as its mode, ownership,
 * size, and data block pointers. An array of these structures (`inode[NINODE]`)
 * serves as the in-core inode cache.
 */
struct inode {
	char	i_flag;		/**< Flags for inode status (see flag codes below). */
	char	i_count;	/**< Reference count (number of active uses of this inode). */
	kernel_dev_t i_dev;	/**< Device where this inode resides. */
	int	i_number;	/**< Inode number on its device. */
	int	i_mode;		/**< File mode (type and permissions - see mode codes below). */
	char	i_nlink;	/**< Number of hard links to this file. */
	char	i_uid;		/**< User ID of the file owner. */
	char	i_gid;		/**< Group ID of the file. */
	char	i_size0;	/**< High-order byte of the file size (for files > 32K words on PDP-11). */
	char	*i_size1;	/**< Low-order word (16 bits) of the file size. This is a char* likely for byte addressing or specific PDP-11 type punning. Effectively part of a 24-bit or 32-bit size. */
	int	i_addr[8];	/**< Array of block numbers. Addr[0]-addr[6] are direct blocks for small files. Addr[7] can be a double indirect block. For large files (ILARG flag), these can be indirect block pointers. */
	int	i_lastr;	/**< Last logical block number read (for read-ahead heuristic). */
} inode[NINODE];		/**< The in-core inode table. @see NINODE in param.h */

/** @name Inode flags (i_flag) */
///@{
#define	ILOCK	01		/**< Inode is locked (e.g., during manipulation). */
#define	IUPD	02		/**< Inode has been modified and needs to be written to disk. */
#define	IACC	04		/**< Inode access time needs to be updated on disk. */
#define	IMOUNT	010		/**< Inode is a mount point. */
#define	IWANT	020		/**< Another process wants to lock this inode. */
#define	ITEXT	040		/**< Inode is for a pure text segment (sharable). */
///@}

/** @name Inode modes (i_mode) - type and permissions */
///@{
#define	IALLOC	0100000	/**< Inode is allocated. */
#define	IFMT	060000	/**< Mask for file type. */
#define		IFDIR	040000	/**< Directory. */
#define		IFCHR	020000	/**< Character special file. */
#define		IFBLK	060000	/**< Block special file. (Note: Same as IFMT, implies it's not IFDIR or IFCHR if this is set after masking) */
							/**< Regular file is indicated by IFMT bits being 000000 after masking. */
#define	ILARG	010000	/**< Large file (uses indirect blocks). */
#define	ISUID	04000	/**< Set user ID on execution. */
#define	ISGID	02000	/**< Set group ID on execution. */
#define ISVTX	01000	/**< Save text image after execution (sticky bit). */
#define	IREAD	0400	/**< Read permission, owner. */
#define	IWRITE	0200	/**< Write permission, owner. */
#define	IEXEC	0100	/**< Execute permission, owner. */
/* Other permission bits (group, other) are typically IREAD>>3, IWRITE>>3, IEXEC>>3, etc. */
///@}

/**
 * @struct dinode
 * @brief Structure of an inode as it appears on disk.
 * This is typically 32 bytes on a PDP-11 Unix system.
 */
struct dinode {
	unsigned short	di_mode;	/**< File mode, type and permissions. */
	char		di_nlink;	/**< Number of links to file. */
	char		di_uid;		/**< User ID of owner. */
	char		di_gid;		/**< Group ID of owner. */
	char		di_size0;	/**< High byte of 24-bit size. */
	unsigned short	di_size1;	/**< Low word of 24-bit size. */
	unsigned short	di_addr[8];	/**< Block addresses of file contents (16 bytes). */
                                        /* Direct blocks, then single, double, triple indirect. */
	unsigned int	di_atime[2];	/**< Time last accessed (low, high words). */
	unsigned int	di_mtime[2];	/**< Time last modified (low, high words). */
};
