#ifndef _EXAMPLE_H_
#define _EXAMPLE_H_

#include <linux/cdev.h>
#include <linux/semaphore.h>

#define EXAMPLE_DEVICE_NODE_NAME "example"
#define EXAMPLE_DEVICE_FILE_NAME "example"
#define EXAMPLE_DEVICE_PROC_NAME "example"
#define EXAMPLE_DEVICE_CLASS_NAME "example"
#define EXAMPLE_MAJOR 0

struct example_dev
{
	struct mutex mutex;
	struct cdev cdev;
	int val;
};

#endif