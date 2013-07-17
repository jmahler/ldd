
/*
 * NAME
 * ----
 *
 * datrw - simplest driver that supports read/write
 *
 * SYNOPSIS
 * --------
 *
 * This is a character driver that supports read/write of
 * a fixed buffer of data.
 *
 * To test if it is working try writing some data and reading
 * it back.  As long as MAX_DATA (128) is not exceeded it should
 * be reproduced.
 *
 *   $ sudo dd if=datrw.c of=/dev/datrw0 bs=128 count=1
 *
 *   $ sudo dd if=/dev/datrw0 of=out1 bs=128 count=1
 *   (out1 should have the data from datrw.c)
 *
 * The read always start from 0 so
 *
 *   $ sudo dd if=datrw.c of=/dev/datrw0 bs=64 count=2
 *
 * would read 64 bytes from the beginning twice.
 * And it behaves similarly for write.
 *
 */

#define DEVICE_NAME "datrw"
#define MAX_DATA 128

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t datrw_major;
static int cdev_add_done;

struct datrw_dev {
	struct cdev cdev;
	char data[MAX_DATA];
} *datrw_devp;

struct class *datrw_class;

int datrw_open(struct inode* inode, struct file* filp)
{
	struct datrw_dev *datrw_devp;

	if (DEBUG) printk(KERN_ALERT "datrw_open()\n");

	datrw_devp = container_of(inode->i_cdev, struct datrw_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = datrw_devp;

	return 0;
}

ssize_t datrw_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct datrw_dev *datrw_devp = filp->private_data;
	size_t cnt;

	if (DEBUG) printk(KERN_ALERT "datrw_read()\n");

	cnt = (count > MAX_DATA) ? MAX_DATA : count;

	if (copy_to_user(buf, (void *) datrw_devp->data, cnt) != 0) {
		return -EIO;
	}

	return cnt;
}

ssize_t datrw_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct datrw_dev *datrw_devp = filp->private_data;
	size_t cnt;

	if (DEBUG) printk(KERN_ALERT "datrw_write()\n");

	cnt = (count > MAX_DATA) ? MAX_DATA : count;

	if (copy_from_user((void *) datrw_devp->data, buf, cnt) != 0) {
		return -EIO;
	}

	return cnt;
}

int datrw_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "datrw_release()\n");

	return 0;
}

struct file_operations datrw_fops = {
	.owner = THIS_MODULE,
	.open = datrw_open,
	.read = datrw_read,
	.write = datrw_write,
	.release = datrw_release,
};

static void datrw_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "datrw_cleanup()\n");

	if (!datrw_class) {
		device_destroy(datrw_class, datrw_major);
		class_destroy(datrw_class);
		datrw_class = NULL;
	}

	if (cdev_add_done) {
		cdev_del(&datrw_devp->cdev);
		cdev_add_done = 0;
	}

	if (!datrw_devp) {
		kfree(datrw_devp);
		datrw_devp = NULL;
	}

	if (datrw_major) {
		unregister_chrdev_region(datrw_major, 1);
		datrw_major = 0;
	}
}

static int __init datrw_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "datrw_init()\n");

	/* defaults, tested by cleanup() */
	datrw_major = 0;
	datrw_class = NULL;
	datrw_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&datrw_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	datrw_devp = kmalloc(sizeof(struct datrw_dev), GFP_KERNEL);
	if (!datrw_devp) {
		printk(KERN_WARNING "Unable to kmalloc datrw_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&datrw_devp->cdev, &datrw_fops);
	datrw_devp->cdev.owner = THIS_MODULE;
	datrw_devp->cdev.ops = &datrw_fops;
	err = cdev_add(&datrw_devp->cdev, datrw_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 0;
	}

	/* populate sysfs entries */
	/* /sys/class/datrw/datrw0/ */
	datrw_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/datrw0 */
	device_create(datrw_class, NULL, datrw_major, NULL, "datrw%d",0);

	return 0;  /* success */

out:
	datrw_cleanup();
	return err;
}

static void __exit datrw_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "datrw_exit()\n");

	datrw_cleanup();
}

module_init(datrw_init);
module_exit(datrw_exit);

MODULE_LICENSE("GPL");
