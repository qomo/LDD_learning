/*************************************************************************
	> File Name: scull.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 14时30分34秒
 ************************************************************************/

#ifndef _SCULL_H
#define _SCULL_H

/*
 * Macros to help debugging
 */
#undef PDEBUG           /* undef it, just in case */
#ifdef SCULL_DEBUG
#   ifdef __KERNEL__
        /* This one if debugging is on, and kernel space */
#       define PDEBUG(fmt, arg...) printk( KERN_DEBUG "scull: " fmt, ## args )
#   else
        /* This one for user space */
#       define PDEBUG(fmt, arg...) fprintf(stderr, fmt, ## args)
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4     /* scull0 through scull3 */
#endif

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "scull_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of SCULL_QUANTUM bytes.
 *
 * The array (quantum-set) is SCULL_QSET long.
 */
#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET 
#define SCULL_QSET 1000
#endif

/*
 * Representation of scull quantum sets.
 */
 struct scull_qset {
     void **data;
     struct scull_qset *next;
 };

 struct scull_dev {
     struct scull_qset *data;   /* Pointer to first quantum set */
     int quantum;               /* the current quantum size */
     int qset;                  /* the current array size */
     unsigned long size;        /* amount of data stored here */
     unsigned int access_key;   /* used by sculluid and scullpriv */
     struct mutex mutex;        /* mutual exclusion semaphore */
     struct cdev cdev;          /* Char device structure */
 };


/*
 * Ioctl definition
 */

/* Use 'k' as magic number */
#define SCULL_IOC_MAGIC 'k'

/* Please use a different 8-bit number in your code */

#define SCULL_IOCRESET  _IO(SCULL_IOC_MAGIC, 0)


/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value 
 * G means "Get": reply by setting through a pointer 
 * Q means "Query": response is on the return value 
 * X means "eXchange": switch G and S atomically 
 * H means "sHift": switch T and Q atomically
 */
#define SCULL_IOCSQUANTUM    _IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOCSQSET      _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOCTQUANTUM   _IO(SCULL_IOC_MAGIC, 3)
#define SCULL_IOCTQSET      _IO(SCULL_IOC_MAGIC, 4)
#define SCULL_IOCGQUANTUM   _IOR(SCULL_IOC_MAGIC, 5, int)
#define SCULL_IOCGQSET      _IOR(SCULL_IOC_MAGIC, 6, int)
#define SCULL_IOCQQUANTUM   _IO(SCULL_IOC_MAGIC, 7)
#define SCULL_IOCQQSET      _IO(SCULL_IOC_MAGIC, 8)
#define SCULL_IOCXQUANTUM   _IOWR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET      _IOWR(SCULL_IOC_MAGIC, 10, int)
#define SCULL_IOCHQUANTUM   _IO(SCULL_IOC_MAGIC, 11)
#define SCULL_IOCHQSET      _IO(SCULL_IOC_MAGIC, 12)

/* 
 * The other entities only have "Tell" and "Query", because they're 
 * not printed in the book, and there's no need to have all six.
 * (The previous stuff was only there to show different ways to do it.)
 */
 #define SCULL_P_IOCTSIZE _IO(SCULL_IOC_MAGIC, 13)
 #define SCULL_P_IOCQSIZE _IO(SCULL_IOC_MAGIC, 14)
/* ... more to come */

#define SCULL_IOC_MAXNR 14

/*
 * Prototypes for shared functions
 */
int     scull_p_init(dev_t dev);

#endif /* SCULL_H */

