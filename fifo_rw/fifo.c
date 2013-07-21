
#define DEVICE_NAME "fifo"
#define MAX_DATA 128

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t fifo_major;
static int cdev_add_done;

struct fifo_dev {
	struct cdev cdev;
} *fifo_devp;

struct class *fifo_class;
struct device *fifo_device;

char fifo[MAX_DATA];
size_t read_ofs;
size_t write_ofs;

int fifo_open(struct inode* inode, struct file* filp)
{
	struct fifo_dev *fifo_devp;

	if (DEBUG) printk(KERN_ALERT "fifo_open()\n");

	fifo_devp = container_of(inode->i_cdev, struct fifo_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = fifo_devp;

	return 0;
}

/*
 * The fifo is implemented using a cirular buffer.
 * It has a read offset (read_ofs) and a write offset (write_ofs).
 * Reads can be performed until it reaches the write offset.
 * And writes can be performed until it reaches the read offset.
 *
 * Read as much data as possible:
 *
 *             W
 *   +---+---+---+
 *   |   |   |   |
 *   +---+---+---+
 *     R
 *     
 *             W
 *   +---+---+---+
 *   |   |   |   |
 *   +---+---+---+
 *         R
 *
 *             W
 *   +---+---+---+
 *   |   |   |   |
 *   +---+---+---+
 *             R
 *
 * Write a new value:
 *
 *     W
 *   +---+---+---+
 *   |   |   |   |
 *   +---+---+---+
 *             R
 */

ssize_t fifo_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	size_t cnt;
	size_t left;

	left = count;

	if (DEBUG) printk(KERN_ALERT "fifo_read(%zu)\n", count);
	if (DEBUG) printk(KERN_ALERT "  offset, read, write: %zu, %zu\n",
										read_ofs, write_ofs);

	while (left && read_ofs != write_ofs) {

		if (write_ofs > read_ofs) {
			cnt = write_ofs - read_ofs;
		} else {
			cnt = MAX_DATA - read_ofs;
		}

		if (cnt > left)
			cnt = left;

		if (DEBUG) printk(KERN_ALERT "  to read: %zu\n", cnt);

		if (copy_to_user(buf, (void *) &fifo[read_ofs], cnt) != 0) {
			return -EIO;
		}

		buf += cnt;
		left -= cnt;
		read_ofs += cnt;

		if (read_ofs == MAX_DATA)
			read_ofs = 0;

		if (DEBUG) printk(KERN_ALERT "  read_ofs: %zu\n", read_ofs);
	}

	return (count - left);
}

ssize_t fifo_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	size_t cnt;
	size_t left;

	if (DEBUG) printk(KERN_ALERT "fifo_write(%zu)\n", count);
	if (DEBUG) printk(KERN_ALERT "  offset, read, write: %zu, %zu\n",
										read_ofs, write_ofs);

	left = count;

	while (left) {

		if ( (write_ofs == (MAX_DATA - 1)) && (0 == read_ofs) ) {
			/* loop end to beginning, full, write_ofs < read_ofs */
			break;
		} else if (write_ofs >= read_ofs) {
			cnt = MAX_DATA - write_ofs;
		} else { // write_ofs < read_ofs
			cnt = (read_ofs - 1) - write_ofs;
			if (0 == cnt)
				break;
		}

		if (cnt > left)
			cnt = left;

		if (DEBUG) printk(KERN_ALERT "  write: %zu\n", cnt);

		if (copy_from_user((void *) &fifo[write_ofs], buf, cnt) != 0) {
			return -EIO;
		}

		buf += cnt;
		left -= cnt;
		write_ofs += cnt;

		/* reset to begining if we reach the end */
		if (write_ofs == MAX_DATA)
			write_ofs = 0;

		if (DEBUG) printk(KERN_ALERT "  write_ofs: %zu\n", write_ofs);
	}

	return (count - left);
}

int fifo_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "fifo_release()\n");

	return 0;
}

struct file_operations fifo_fops = {
	.owner = THIS_MODULE,
	.open = fifo_open,
	.read = fifo_read,
	.write = fifo_write,
	.release = fifo_release,
};

static void fifo_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "fifo_cleanup()\n");

	if (fifo_major) {
		if (DEBUG) printk(KERN_ALERT "fifo: unregister_chrdev_region()\n");
		unregister_chrdev_region(fifo_major, 1);
	}

	if (fifo_device) {
		if (DEBUG) printk(KERN_ALERT "fifo: device_destroy()\n");
		device_destroy(fifo_class, fifo_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "fifo: cdev_del()\n");
		cdev_del(&fifo_devp->cdev);
	}

	if (fifo_devp) {
		if (DEBUG) printk(KERN_ALERT "fifo: kfree()\n");
		kfree(fifo_devp);
	}

	if (fifo_class) {
		if (DEBUG) printk(KERN_ALERT "fifo: class_destroy()\n");
		class_destroy(fifo_class);
	}
}

static int __init fifo_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "fifo_init()\n");

	read_ofs = 0;
	write_ofs = 0;

	/* defaults, tested by cleanup() */
	fifo_major = 0;
	fifo_class = NULL;
	fifo_device = NULL;
	fifo_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&fifo_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/fifo/fifo0/ */
	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);

	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		printk(KERN_WARNING "Unable to kmalloc fifo_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&fifo_devp->cdev, fifo_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/fifo0 */
	fifo_device = device_create(fifo_class, NULL, MKDEV(MAJOR(fifo_major), 0), NULL, "fifo%d",0);

	return 0;  /* success */

out:
	fifo_cleanup();
	return err;
}

static void __exit fifo_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "fifo_exit()\n");

	fifo_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(fifo_init);
module_exit(fifo_exit);

