/*
 *	Copyright 1973 Bell Telephone Laboratories Inc
 */

#include <stdio.h>  /* For printf */
#include <stdlib.h> /* For NULL */
#include <string.h> /* For bcopy (or potentially memcpy/memmove if bcopy is not ideal) */

#include "../param.h"  /* For BSIZE, ENOSPC, NODEV, NINODE, NMOUNT, PINOD, rootdev, kernel_dev_t etc. */
#include "../systm.h"  /* For time, updlock, struct mount, etc. */
#include "../filsys.h" /* Not directly used, but good for context */
#include "../file.h"   /* For struct file, file[], FREAD, FWRITE, FPIPE, NOFILE, NFILE */
#include "../conf.h"   /* For struct cdevsw, bdevsw, nchrdev, nblkdev */
#include "../inode.h"  /* For struct inode, ILOCK, IUPD, etc. */
#include "../reg.h"    /* For R0 (used in u.u_ar0[R0]) */


/* --- Forward Declarations --- */
/* from bio.c */
extern struct buf *bread(kernel_dev_t dev, int blkno);
extern struct buf *getblk(kernel_dev_t dev, int blkno);
extern void brelse(struct buf *bp);
extern void bwrite(struct buf *bp);

/* from iget.c / nami.c / subr.c */
extern struct inode *iget(kernel_dev_t dev, int ino);
extern void iput(struct inode *ip);
extern void iupdat(struct inode *ip, int *time_arr); /* timep is int time[2] */
extern void prele(struct inode *ip);
extern struct inode *namei(int (*func)(void), int flag); /* func gets char from path */
extern int schar(void); /* function for namei to get char from u.u_dirp */

/* from alloc.c */
extern struct filsys *getfs(kernel_dev_t dev);

/* from slp.c */
extern void sleep(void *chan, int pri);
extern void wakeup(void *chan);

/* from prf.c - panic is declared here, printf comes from stdio.h */
extern void panic(const char *s);

/* from main.c or machdep.c - suser is declared here, bcopy from string.h */
extern int suser(void);

/* Static forward declarations for functions local to this file */
static void closei(struct inode *ip, int rw);
static void openi(struct inode *ip, int rw);


/* --- Global Variables --- */
struct file *maxfp; /* Max file pointer used, for some accounting? (Original V5 had this) */

/* --- Function Definitions --- */

/**
 * @brief Get a pointer to the system file table entry for a given file descriptor.
 * @param f The file descriptor number.
 * @return Pointer to the struct file, or NULL if fd is invalid or file not open (u.u_error set).
 */
struct file *getf(int f)
{
	struct file *fp;

	if(f < 0 || f >= NOFILE) { /* Check if file descriptor is out of bounds */
		u.u_error = EBADF;
		return(NULL);
	}
	fp = u.u_ofile[f];
	if(fp == NULL) { /* Check if the file descriptor slot is empty */
		u.u_error = EBADF;
		/* fp is already NULL */
	}
	return(fp);
}

/**
 * @brief Close a file table entry. Decrements reference count and calls closei if last reference.
 * @param fp Pointer to the file structure to close.
 */
void closef(struct file *fp)
{
	struct inode *ip;

	if(fp == NULL) { /* Should not happen if called correctly */
        return;
    }

	if(fp->f_flag & FPIPE) {
		ip = fp->f_inode;
        if (ip) { /* Should always be true for a pipe */
		    ip->i_mode &= ~(IREAD|IWRITE); /* Clear pipe read/write modes */
		    wakeup((void*)(ip+1)); /* Wakeup readers (inode address + 1 often for read chan) */
		    wakeup((void*)(ip+2)); /* Wakeup writers (inode address + 2 often for write chan) */
        }
	}

	if(fp->f_count <= 1) { /* Last reference */
		closei(fp->f_inode, (fp->f_flag & FWRITE));
        fp->f_count = 0; /* Explicitly zero, though closei might iput which also decs */
	} else {
	    fp->f_count--;
    }
    /* u.u_ofile entry pointing to this fp should be cleared by caller (e.g. close syscall) */
}

/**
 * @brief Close an inode. Called when the last reference to an open file is closed.
 * Handles device-specific close operations for character and block special files.
 * @param ip Pointer to the in-core inode.
 * @param rw Flag indicating if the file was open for writing (relevant for some devices).
 */
static void closei(struct inode *ip, int rw)
{
	kernel_dev_t dev;
	int maj;

	if(ip == NULL) return;

	if(ip->i_count <= 1) { /* Should be 1 if this is the last closef */
		switch(ip->i_mode & IFMT) {
		case IFCHR:
			dev = ip->i_dev;
			maj = major(dev);
            if ((unsigned)maj < nchrdev && cdevsw[maj].d_close) {
			    (*cdevsw[maj].d_close)(dev, rw);
            }
			break;

		case IFBLK:
			dev = ip->i_dev;
			maj = major(dev);
            if ((unsigned)maj < nblkdev && bdevsw[maj].d_close) {
			    (*bdevsw[maj].d_close)(dev, rw);
            }
			break;
		}
	}
	iput(ip); /* Decrement inode reference count, potentially freeing it */
}

/**
 * @brief Open an inode. Handles device-specific open operations.
 * @param ip Pointer to the in-core inode.
 * @param rw Read/write mode flags.
 */
static void openi(struct inode *ip, int rw)
{
	kernel_dev_t dev;
	int maj;

	if(ip == NULL) {
        u.u_error = ENOENT; /* Or some other suitable error */
        return;
    }
	dev = ip->i_dev;
	maj = major(dev);

	switch(ip->i_mode & IFMT) {
	case IFCHR:
		if((unsigned)maj >= nchrdev) { /* Check against declared number of char device types */
			goto bad_device;
        }
        if (cdevsw[maj].d_open) {
		    (*cdevsw[maj].d_open)(dev, rw);
        }
		break;

	case IFBLK:
		if((unsigned)maj >= nblkdev) { /* Check against declared number of block device types */
		bad_device:
			u.u_error = ENXIO; /* No such device or address */
			return;
		}
        if (bdevsw[maj].d_open) {
		    (*bdevsw[maj].d_open)(dev, rw);
        }
		break;
	}
}

/**
 * @brief Check mode permission on an inode.
 * @param aip Pointer to the in-core inode.
 * @param mode The mode to check (IREAD, IWRITE, IEXEC).
 * @return 0 if access is allowed, 1 if not (u.u_error is set).
 */
int access(struct inode *aip, int mode)
{
	int m;
    struct filsys *fs_p;

	if (aip == NULL) {
        u.u_error = EFAULT; /* Should not happen if called with valid inode */
        return 1;
    }
	m = mode;
	if(m == IWRITE) {
        fs_p = getfs(aip->i_dev);
        if (fs_p && fs_p->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if(aip->i_flag & ITEXT) { /* Cannot write to an active text segment */
			u.u_error = ETXTBSY;
			return(1);
		}
	}

	if(u.u_uid == 0) { /* Superuser */
		if(m == IEXEC && (aip->i_mode & (IEXEC | (IEXEC>>3) | (IEXEC>>6))) == 0) {
            /* Superuser still needs at least one execute bit set for execute permission */
			u.u_error = EACCES;
            return(1);
        }
		return(0); /* Superuser allowed otherwise */
	}

	if(u.u_uid != aip->i_uid) { /* Not owner */
		m >>= 3; /* Shift to group permissions */
		if(u.u_gid != aip->i_gid) { /* Not in group */
			m >>= 3; /* Shift to other permissions */
        }
	}

	if((aip->i_mode & m) != 0) { /* Check if the required permission bits are set */
		return(0); /* Access granted */
    }

	u.u_error = EACCES;
	return(1); /* Access denied */
}

/**
 * @brief Get inode of file owned by current user. Used by chown/chmod.
 * @return Pointer to the inode if successful and owned/superuser, else NULL.
 */
struct inode *owner(void)
{
	struct inode *ip;

	if ((ip = namei(schar, 0)) == NULL) {
		return(NULL);
    }
	if(u.u_uid == ip->i_uid) { /* Is owner */
		return(ip);
    }
	if (suser()) { /* Is superuser */
		return(ip);
    }
	iput(ip); /* Not owner and not superuser, release inode */
	return(NULL);
}

/* suser() is typically in main.c or machdep.c, assumed extern here */

/**
 * @brief Allocate a user file descriptor.
 * Finds the first free slot in the current process's open file table (u.u_ofile).
 * @return The file descriptor index, or -1 on error (u.u_error = EMFILE).
 */
int ufalloc(void)
{
	register int i;

	for (i=0; i<NOFILE; i++) {
		if (u.u_ofile[i] == NULL) {
			u.u_ar0[R0] = i; /* Return fd in R0 for syscalls */
			return(i);
		}
    }
	u.u_error = EMFILE; /* Too many open files for this process */
	return(-1);
}


/**
 * @brief Allocate an entry in the system open file table (file[]).
 * Finds a free entry, associates it with a user file descriptor, and initializes it.
 * @return Pointer to the allocated struct file, or NULL on error (u.u_error set).
 */
struct file *falloc(void)
{
	struct file *fp;
	register int i;

	i = ufalloc(); /* Get a free file descriptor for the current process */
	if (i < 0) { /* ufalloc sets u.u_error */
		return(NULL);
    }

	for (fp = &file[0]; fp < &file[NFILE]; fp++) {
		if (fp->f_count==0) { /* Found a free entry in system file table */
			u.u_ofile[i] = fp; /* Link user fd to this system file entry */
			fp->f_count++;
			fp->f_offset[0] = 0; /* Initialize offset */
			fp->f_offset[1] = 0;
			/* maxfp was a V5 global, purpose unclear, perhaps for debugging or stats.
			 * It's not typically essential for core functionality.
			 */
			/* if (fp>maxfp) maxfp = fp; */
			return(fp);
		}
    }
    /* No free entry in system file table */
	printf("System file table overflow\n"); /* Should be a panic or error to user */
	u.u_ofile[i] = NULL; /* Un-do the ufalloc assignment */
	u.u_error = ENFILE;  /* System file table overflow */
	return(NULL);
}
