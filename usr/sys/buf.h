/**
 * @file buf.h
 * @brief Definition of the buffer cache header (`struct buf`) and related structures/constants.
 *
 * This file defines the structure of buffer headers used by the system for
 * block I/O caching. It also defines the device I/O queue structure (`struct devtab`)
 * and flags associated with buffer states.
 */

// Forward declaration for self-reference
struct buf;
struct devtab; // Though devtab itself uses struct buf pointers, not vice-versa directly in struct buf

/**
 * @struct buf
 * @brief Buffer header structure for block I/O.
 *
 * Each `struct buf` represents a system buffer that can hold one block of data
 * from a block device. These buffers are managed in a cache and are also used
 * for device I/O queues.
 */
struct buf {
	int	b_flags;		/**< Buffer status flags (see B_ flags below). */
	struct	buf *b_forw;	/**< Pointer to next buffer in hash queue or free list. */
	struct	buf *b_back;	/**< Pointer to previous buffer in hash queue or free list. */
	struct	buf *av_forw;	/**< Pointer to next buffer in device active queue (driver's request list). */
	struct	buf *av_back;	/**< Pointer to previous buffer in device active queue. */
	kernel_dev_t b_dev;		/**< Device number (major and minor). */
	int	b_wcount;		/**< Negative of number of words to transfer (historical usage, often just word count). */
	char	*b_addr;		/**< Core address of the data buffer itself (512 bytes). */
	char	*b_blkno;		/**< Block number on device (char* for PDP-11 large address space arithmetic, effectively a long). */
	char	b_error;		/**< Error number if I/O error occurred. */
	char	*b_resid;		/**< Remaining char count on error (or words). */
} buf[NBUF];				/**< The array of system buffers. @see NBUF in param.h */

/**
 * @struct devtab
 * @brief Device I/O queue header structure (per block device).
 *
 * This structure manages the queue of I/O requests for a block device.
 * The `b_forw` and `b_back` fields are used to link this header into a list
 * of device tabs by some drivers, but more commonly `d_actf` and `d_actl`
 * are used for the actual I/O request queue.
 * The comment "forw and back are shared with 'buf' struct" is a bit misleading;
 * it means this struct is often part of a device driver's status structure which
 * also includes buffer pointers or that its queue pointers link `struct buf`.
 */
struct devtab {
	char	d_active;		/**< Flag indicating if device is currently active with an I/O operation. */
	char	d_errcnt;		/**< Count of errors on recent I/O operations. */
	struct	buf *b_forw;	/**< Unused by modern interpretation or for specific driver list linking. */
	struct	buf *b_back;	/**< Unused by modern interpretation or for specific driver list linking. */
	struct	buf *d_actf;	/**< Pointer to first buffer in device's active I/O queue. */
	struct 	buf *d_actl;	/**< Pointer to last buffer in device's active I/O queue. */
};

extern struct buf bfreelist;	/**< Head of the buffer free list. */

/** @name Buffer flags (b_flags) */
///@{
#define	B_WRITE	 0		/**< Write operation (historically 0, B_READ is 1, so !B_READ implies write). */
#define	B_READ	 01		/**< Read operation. */
#define	B_DONE	 02		/**< I/O operation completed. */
#define	B_ERROR	 04		/**< I/O operation resulted in an error. */
#define	B_BUSY	 010	/**< Buffer is currently busy (e.g., I/O in progress, or locked). */
#define	B_XMEM	 060	/**< Extended memory transfer (PDP-11/45+ specific for >64KB memory). Bits for Unibus map. */
#define	B_WANTED 0100	/**< Process is waiting for this buffer. */
#define	B_RELOC	 0200	/**< Buffer is for a relocation segment (?). */
#define	B_ASYNC	 0400	/**< Asynchronous I/O (don't wait for completion). */
#define	B_DELWRI 01000	/**< Delayed write (buffer is dirty, write later). */
///@}
