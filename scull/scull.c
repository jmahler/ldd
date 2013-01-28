
/*
 * This "scull" driver is derived from the example given
 * in the book "Linux Device Drivers", 3rd edition, by
 * Jonathan Corbet, et. al.
 */

#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>

int scull_major =   0;
int scull_nr_devs = 4;
int scull_quantum = 4000;
int scull_qset =    1000;

module_param(scull_major, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;
	int quantum;
	int qset;
	unsigned long size;
	unsigned int access_key;
	struct semaphore sem;
	struct cdev cdev;
};

struct scull_dev *scull_devices;
struct class *scull_class;

// {{{ scull_trim
int scull_trim(struct scull_dev *dev)
{
	struct scull_qset *next, *dptr;
	int qset = dev->qset;  /* "dev" is not-null */
	int i;

	// go through the linked list and free all memory
	for (dptr = dev->data; dptr; dptr = next) {
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);

			kfree(dptr->data);
			dptr->data = NULL;
		}

		next = dptr->next;
		kfree(dptr);
	}

	dev->size = 0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;

	return 0;
}
// }}}

// {{{ scull_follow
struct scull_qset*
scull_follow(struct scull_dev *dev, int n)
{
        struct scull_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
        if (! qs) {
                qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
                if (qs == NULL)
                        return NULL;  /* Never mind */
                memset(qs, 0, sizeof(struct scull_qset));
        }

        /* Then follow the list */
        while (n--) {
                if (!qs->next) {
                        qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
                        if (qs->next == NULL)
                                return NULL;  /* Never mind */
                        memset(qs->next, 0, sizeof(struct scull_qset));
                }
                qs = qs->next;
                continue;
        }
        return qs;
}
// }}}

// {{{ scull_read
ssize_t
scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;  // the first listitem
	int quantum, qset;
	int itemsize;
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	quantum = dev->quantum;
	qset = dev->qset;
	itemsize = quantum * qset;

	// adjust count to a realistic value
	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	// find listitem qset index, and offset in the quantum
	item = (long) *f_pos / itemsize;
	rest = (long) *f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	// follow the list up to the right position (defined elsewhere)
	dptr = scull_follow(dev, item);

	if (NULL == dptr || !dptr->data || !dptr->data[s_pos])
		goto out;  // don't fill holes

	// read only up to the end of this quantum
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

out:
	up(&dev->sem);

	return retval;
}
// }}}

// {{{ scull_write
ssize_t
scull_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; // value used in "goto out"

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* find listitem, qset index and offset in the quantum */
	item = (long) *f_pos / itemsize;
	rest = (long) *f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = scull_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

	/* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

out:
	up(&dev->sem);
	return retval;
}
// }}}

// {{{ scull_open
int
scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(dev);
	}

	return 0;  // OK
}
// }}}

// {{{ scull_release

int
scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}
// }}}

struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.read = scull_read,
	.write = scull_write,
	.open = scull_open,
	.release = scull_release,
};

// {{{ scull_setup_cdev
static void
scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err;
	int devno = MKDEV(scull_major, index);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;

	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}
// }}}

// {{{ scull_cleanup
void scull_cleanup(void)
{
	int i;

	unregister_chrdev_region(MKDEV(scull_major, 0),
					scull_nr_devs);

	if (scull_class) {
		for (i = 0; i < scull_nr_devs; i++) {
			device_destroy(scull_class, MKDEV(scull_major, i));
		}
	}

	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			scull_trim(scull_devices + i);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}

	if (scull_class) {
		class_destroy(scull_class);
	}
}
// }}}

// {{{ scull_init

int
scull_init(void)
{
	int result;
	int i;
	dev_t dev = 0;

	// setup the major and minor numbers
	if (scull_major) {
		dev = MKDEV(scull_major, 0);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
		result = alloc_chrdev_region(&dev, 0, scull_nr_devs,
					"scull");
		scull_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	// allocate the devices
	// This can't be static because the number can change at
	// load time.
	scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev),
					GFP_KERNEL);
	if (!scull_devices) {
		result = -ENOMEM;
		goto fail;
	}

	memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

	/* Initialize each device */
	for (i = 0; i < scull_nr_devs; i++) {
		scull_devices[i].quantum = scull_quantum;
		scull_devices[i].qset = scull_qset;
		sema_init(&scull_devices[i].sem, 1);
		scull_setup_cdev(&scull_devices[i], i);
	}

	scull_class = class_create(THIS_MODULE, "scull");
	if (IS_ERR(scull_class)) {
		scull_class = 0;  // class is void
		printk(KERN_ERR "Unable to create scull class\n");
		goto fail;
	}

	for (i = 0; i < scull_nr_devs; i++) {
		device_create(scull_class, NULL, MKDEV(scull_major, i),
					NULL, "scull%d", i);
	}

	return 0;  // OK

fail:
	scull_cleanup();

	return result;
}

// }}}

module_init(scull_init);
module_exit(scull_cleanup);

MODULE_LICENSE("GPL");
