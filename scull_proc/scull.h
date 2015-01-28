/*************************************************************************
	> File Name: scull.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月23日 星期五 14时30分34秒
 ************************************************************************/

#ifndef _SCULL_H
#define _SCULL_H

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

#endif /* SCULL_H */

