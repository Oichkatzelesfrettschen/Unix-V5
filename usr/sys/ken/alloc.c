/*
 *	Copyright 1973 Bell Telephone Laboratories Inc
 */

#include <stdio.h>  /* For printf */
#include <stdlib.h> /* For NULL */
#include <string.h> /* For bcopy (or potentially memcpy/memmove if bcopy is not ideal) */

#include "../param.h"  /* For BSIZE, ENOSPC, NODEV, NINODE, NMOUNT, PINOD, rootdev, kernel_dev_t etc. */
#include "../systm.h"  /* For time, updlock, struct mount, etc. */
#include "../filsys.h" /* For struct filsys */
#include "../buf.h"    /* For struct buf */
#include "../inode.h"  /* For struct inode, struct dinode */
#include "../user.h"   /* For u struct (u.u_error) */


/* --- Forward Declarations for functions potentially used by or defined in alloc.c --- */

/* Functions from bio.c (or similar block I/O module) */
struct buf *bread(kernel_dev_t dev, int blkno);
struct buf *getblk(kernel_dev_t dev, int blkno);
void brelse(struct buf *bp);
void bwrite(struct buf *bp);
void bflush(kernel_dev_t dev);
void clrbuf(struct buf *bp);

/* Functions from iget.c / nami.c / subr.c */
struct inode *iget(kernel_dev_t dev, int ino);
void iput(struct inode *ip);
void iupdat(struct inode *ip, int *time_arr);
void prele(struct inode *ip);

/* Functions from slp.c (process context) */
void sleep(void *chan, int pri);
void wakeup(void *chan);

/* Functions from prf.c (kernel printf/panic) */
void panic(const char *s);
void prdev(const char *msg, kernel_dev_t dev);

/* Functions from main.c or other general kernel util */
/* void bcopy(const void *src, void *dst, int count); // Assuming string.h or kernel version */


/* --- Function Definitions --- */

/**
 * @brief Initialize the in-core superblock for the root device.
 * Called once during system startup from main().
 */
void iinit(void)
{
	struct buf *bp_root_sb;
	struct buf *bp_mount_sb;
	struct filsys *fs_p;

	bp_root_sb = bread(rootdev, 1);
	if (u.u_error) {
		panic("iinit: root sb read error");
	}

	bp_mount_sb = getblk(NODEV, 0);

	if (u.u_error) {
        brelse(bp_root_sb);
		panic("iinit: getblk for mount table failed");
	}

	bcopy((void*)bp_root_sb->b_addr, (void*)bp_mount_sb->b_addr, (int)sizeof(struct filsys));
	brelse(bp_root_sb);

	mount[0].m_bufp = bp_mount_sb;
	mount[0].m_dev = rootdev; /* rootdev should be kernel_dev_t */

	fs_p = (struct filsys *)bp_mount_sb->b_addr;
	fs_p->s_flock = 0;
	fs_p->s_ilock = 0;
	fs_p->s_ronly = 0;
}

/**
 * @brief Get the in-core superblock structure for a device.
 * @param dev Device number (kernel_dev_t).
 * @return Pointer to the in-core filsys structure, or panics if not found.
 */
struct filsys *getfs(kernel_dev_t dev)
{
	struct mount *mp_iter;
	struct filsys *fs_ptr;
    int nfree_val, ninode_val;

	for(mp_iter = &mount[0]; mp_iter < &mount[NMOUNT]; mp_iter++) {
		if(mp_iter->m_bufp != NULL && mp_iter->m_dev == dev) {
			fs_ptr = (struct filsys *)mp_iter->m_bufp->b_addr;
			nfree_val = fs_ptr->s_nfree;
			ninode_val = fs_ptr->s_ninode;
			if(nfree_val > 100 || nfree_val < 0 || ninode_val > 100 || ninode_val < 0) {
				prdev("bad count in getfs", dev);
				fs_ptr->s_nfree = 0;
				fs_ptr->s_free[0] = 0;
				fs_ptr->s_ninode = 0;
				fs_ptr->s_inode[0] = 0;
                fs_ptr->s_fmod = 1;
			}
			return(fs_ptr);
		}
    }
	panic("no fs for getfs");
	return(NULL);
}


/**
 * @brief Check if a block number is valid for the filesystem.
 * @param afp Pointer to the filesystem's superblock.
 * @param abn Block number to check.
 * @param dev Device number (kernel_dev_t for error reporting).
 * @return 1 if bad (invalid), 0 if good (valid).
 */
int badblock(struct filsys *afp, int abn, kernel_dev_t dev)
{
	if (abn < afp->s_isize + 2 || abn >= afp->s_fsize) {
		prdev("bad block", dev);
		return(1);
	}
	return(0);
}

/**
 * @brief Allocate a free disk block from the specified device.
 * @param dev The device (kernel_dev_t) from which to allocate.
 * @return Pointer to a buffer containing the allocated block (cleared), or NULL on error (u.u_error set).
 */
struct buf *alloc(kernel_dev_t dev)
{
	int bno;
	struct buf *bp;
    int *free_list_block_data;
	struct filsys *fp;

	fp = getfs(dev);
	if (fp == NULL) {
        return NULL;
    }

	while(fp->s_flock) {
		sleep((void*)&fp->s_flock, PINOD);
    }

	do {
		if (fp->s_nfree <= 0) {
			prdev("alloc: s_nfree <= 0", dev);
            fp->s_nfree = 0;
			u.u_error = ENOSPC;
			return(NULL);
		}
		bno = fp->s_free[--fp->s_nfree];
		if(bno == 0) {
			fp->s_nfree++;
			prdev("no space", dev);
			u.u_error = ENOSPC;
			return(NULL);
		}
	} while (badblock(fp, bno, dev));

	if(fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno);
		if (u.u_error) {
            fp->s_flock = 0;
            wakeup((void*)&fp->s_flock);
            prdev("alloc: bread failed for free list refill", dev);
            return NULL;
        }
		free_list_block_data = (int *)bp->b_addr;
		fp->s_nfree = *free_list_block_data++;
		if(fp->s_nfree > 100 || fp->s_nfree < 0) {
            prdev("alloc: corrupt free list block count", dev);
            fp->s_nfree = 0;
        }
		if (fp->s_nfree > 0) {
            bcopy((void*)free_list_block_data, (void*)fp->s_free, fp->s_nfree * sizeof(int));
        }
		brelse(bp);
		fp->s_flock = 0;
		wakeup((void*)&fp->s_flock);
	}

	bp = getblk(dev, bno);
	if (u.u_error) {
        prdev("alloc: getblk failed for allocated block", dev);
        return NULL;
    }
	clrbuf(bp);
	fp->s_fmod = 1;
	return(bp);
}

/**
 * @brief Free a disk block on the specified device. (Renamed from original 'free')
 * @param dev Device number (kernel_dev_t).
 * @param bno Block number to free.
 */
void bfree_alloc_c(kernel_dev_t dev, int bno)
{
	struct filsys *fp;
	struct buf *bp;
    int *free_list_block_data;

	fp = getfs(dev);
    if (fp == NULL) return;

	fp->s_fmod = 1;

	while(fp->s_flock) {
		sleep((void*)&fp->s_flock, PINOD);
    }

	if (badblock(fp, bno, dev)) {
		return;
    }

	if(fp->s_nfree >= 100) {
		fp->s_flock++;
		bp = getblk(dev, bno);
		if (u.u_error) {
             fp->s_flock = 0;
             wakeup((void*)&fp->s_flock);
             prdev("bfree_alloc_c: getblk failed for block to be freed", dev);
             return;
        }
		free_list_block_data = (int *)bp->b_addr;
		*free_list_block_data++ = fp->s_nfree;
		bcopy((void*)fp->s_free, (void*)free_list_block_data, 100 * sizeof(int));
		fp->s_nfree = 0;
		bwrite(bp);
		fp->s_flock = 0;
		wakeup((void*)&fp->s_flock);
	}
	fp->s_free[fp->s_nfree++] = bno;
	fp->s_fmod = 1;
}


/**
 * @brief Allocate a free inode on the specified device.
 * @param dev Device number (kernel_dev_t).
 * @return Pointer to the locked in-core inode, or NULL on error (u.u_error set).
 */
struct inode *ialloc(kernel_dev_t dev)
{
	struct filsys *fp;
	struct buf *bp;
	struct dinode *dp_disk_array;
    struct inode *ip_incore_allocated;
	int i, j, k, current_ino_num;

	fp = getfs(dev);
    if (fp == NULL) return NULL;

	while(fp->s_ilock) {
		sleep((void*)&fp->s_ilock, PINOD);
    }
loop:
	if(fp->s_ninode > 0) {
		current_ino_num = fp->s_inode[--fp->s_ninode];
		ip_incore_allocated = iget(dev, current_ino_num);

		if (ip_incore_allocated == NULL) {
            if (fp->s_ninode < 100) fp->s_inode[fp->s_ninode++] = current_ino_num;
            goto loop;
        }

		if(ip_incore_allocated->i_mode == 0) {
			for(k=0; k < 8; k++) ip_incore_allocated->i_addr[k] = 0;
			fp->s_fmod = 1;
			return(ip_incore_allocated);
		}
		printf("ialloc: busy i %d on dev %d (mode %o)\n", current_ino_num, dev, ip_incore_allocated->i_mode);
		iput(ip_incore_allocated);
		goto loop;
	}

	fp->s_ilock++;
	current_ino_num = 0;
	for(i=0; i < fp->s_isize; i++) {
		bp = bread(dev, i+2);
		if (u.u_error) {
            if(bp) brelse(bp);
            fp->s_ilock = 0;
            wakeup((void*)&fp->s_ilock);
            prdev("ialloc: bread error reading inode block", dev);
            return NULL;
        }
		dp_disk_array = (struct dinode *)bp->b_addr;
		for(j=0; j < (BSIZE/sizeof(struct dinode)); j++) {
			current_ino_num++;
			if(dp_disk_array[j].di_mode != 0) {
				continue;
            }
			for(k=0; k<NINODE; k++) {
				if(inode[k].i_count > 0 && inode[k].i_dev==dev && inode[k].i_number==current_ino_num) {
					goto cont;
                }
            }
			if (fp->s_ninode < 100) {
                fp->s_inode[fp->s_ninode++] = current_ino_num;
            } else {
                break;
            }
		cont:;
		}
		brelse(bp);
		if(fp->s_ninode >= 100) {
			break;
        }
	}

	fp->s_ilock = 0;
	wakeup((void*)&fp->s_ilock);

	if(fp->s_ninode <= 0) {
		prdev("out of inodes", dev);
		u.u_error = ENOSPC;
		/* panic("out of inodes"); // Original panics */
		return(NULL);
	}
	goto loop;
}

/**
 * @brief Free an in-core inode. (Called by iput when link count is zero)
 * Adds the inode number to the free inode cache in the superblock.
 * @param dev Device (kernel_dev_t) of the inode.
 * @param ino Inode number to free.
 */
void ifree(kernel_dev_t dev, int ino)
{
	struct filsys *fp;

	fp = getfs(dev);
    if (fp == NULL) return;

	if(fp->s_ilock) {
		return;
    }
	if(fp->s_ninode >= 100) {
		return;
    }

	fp->s_inode[fp->s_ninode++] = ino;
	fp->s_fmod = 1;
}


/**
 * @brief Update superblocks and inodes on disk (sync system call).
 * Iterates through mounted filesystems and active inodes, writing out modified ones.
 */
void update(void)
{
	struct inode *ip_iter;
	struct mount *mp_iter;
	struct buf *bp_sb;
    struct filsys *fs_ptr;

	if(updlock) {
		return;
    }
	updlock++;

	for(mp_iter = &mount[0]; mp_iter < &mount[NMOUNT]; mp_iter++) {
		if(mp_iter->m_bufp != NULL) {
			fs_ptr = (struct filsys *)mp_iter->m_bufp->b_addr;
			if(fs_ptr->s_fmod == 0 || fs_ptr->s_ilock != 0 ||
			   fs_ptr->s_flock != 0 || fs_ptr->s_ronly != 0) {
				continue;
            }
			bp_sb = getblk(mp_iter->m_dev, 1);
            if (u.u_error) {
                prdev("update: getblk for superblock failed", mp_iter->m_dev);
                continue;
            }
			fs_ptr->s_fmod = 0;
			fs_ptr->s_time[0] = time[0];
			fs_ptr->s_time[1] = time[1];
			bcopy((void*)fs_ptr, bp_sb->b_addr, (int)sizeof(struct filsys));
			bwrite(bp_sb);
		}
    }

	for(ip_iter = &inode[0]; ip_iter < &inode[NINODE]; ip_iter++) {
		if(ip_iter->i_count > 0 && (ip_iter->i_flag & ILOCK) == 0) {
			ip_iter->i_flag |= ILOCK;
			iupdat(ip_iter, time);
			prele(ip_iter);
		}
    }

	updlock = 0;
	bflush(NODEV);
}
