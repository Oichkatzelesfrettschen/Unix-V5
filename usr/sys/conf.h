/**
 * @file conf.h
 * @brief Definitions for device switch tables (bdevsw, cdevsw).
 */

/**
 * @struct devicedesc
 * @brief Structure to represent a device number (major and minor).
 *
 * This anonymous structure is typically used to access the major and minor
 * parts of a 16-bit device number (dev_t) on the PDP-11, where major usually
 * indicates the driver and minor indicates the specific unit or sub-device.
 * The byte order (lobyte for minor, hibyte for major, or vice-versa)
 * would depend on the PDP-11's endianness (little-endian) and how dev_t is packed.
 * Assuming dev_t is a 16-bit int: `(major << 8) | minor`.
 */
#include "../buf.h" /* Provides struct buf and struct devtab definitions */

struct
{
	char	d_minor;	/**< Minor device number. */
	char	d_major;	/**< Major device number. */
};

/**
 * @struct bdevsw
 * @brief Block device switch table entry structure.
 *
 * Each entry in `bdevsw[]` defines the interface routines for a block device driver.
 */
struct	bdevsw
{
	int	(*d_open)(kernel_dev_t dev, int flag);	/**< Pointer to device open function. */
	int	(*d_close)(kernel_dev_t dev, int flag);	/**< Pointer to device close function. */
	int	(*d_strategy)(struct buf *bp);       /**< Pointer to device strategy function (bp contains bp->b_dev as kernel_dev_t). */
	struct devtab *d_tab;	                    /**< Pointer to device's I/O queue header (struct devtab in buf.h). */
} bdevsw[];					/**< The block device switch table. */
extern int	nblkdev;		/**< Number of entries in bdevsw (number of block device types). */

/**
 * @struct cdevsw
 * @brief Character device switch table entry structure.
 *
 * Each entry in `cdevsw[]` defines the interface routines for a character device driver.
 */
struct	cdevsw
{
	int	(*d_open)(kernel_dev_t dev, int flag);	/**< Pointer to device open function. */
	int	(*d_close)(kernel_dev_t dev, int flag);	/**< Pointer to device close function. */
	int	(*d_read)(kernel_dev_t dev);	        /**< Pointer to device read function. */
	int	(*d_write)(kernel_dev_t dev);	        /**< Pointer to device write function. */
	int	(*d_sgtty)(kernel_dev_t dev, int arg, int mode);	/**< Pointer to device control function (stty/gtty). */
} cdevsw[];					/**< The character device switch table. */
extern int	nchrdev;		/**< Number of entries in cdevsw (number of character device types). */
