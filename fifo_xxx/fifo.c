
#define DEVICE_NAME "fifo"
#define MAX_DATA 3

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static int DEBUG = 0;
module_param(DEBUG, int, S_IRUGO);

static dev_t fifo_major;
static int cdev_add_done;

struct fifo_dev {
	struct cdev cdev;
} *fifo_devp;

struct class *fifo_class;
struct device *fifo_device;

char fifo[MAX_DATA];
char *read_ptr;
char *write_ptr;
char *fifo_start;
char *fifo_end;

static DEFINE_MUTEX(read_mtx);
static DEFINE_MUTEX(write_mtx);

#define HALF_SEC 500  /* ms */

/* exaggerate race conditions by waiting for multiple processes */
int in, mid, out;
static void pwait(struct mutex mtx) {

	// queue any threads beyond two
	mutex_lock(&mtx);
	in++;
	mutex_unlock(&mtx);
	while (1) {
		mutex_lock(&mtx);
		if (in <= 2) {
			mutex_unlock(&mtx);
			break;
		}
		mutex_unlock(&mtx);
		msleep(HALF_SEC);
	}

	// wait for two threads to queue up
	mutex_lock(&mtx);
	mid++;
	mutex_unlock(&mtx);
	while (1) {
		mutex_lock(&mtx);
		if (mid == 2) {
			mutex_unlock(&mtx);
			break;
		}
		mutex_unlock(&mtx);
		msleep(HALF_SEC);
	}

	// wait two threads to leave, reset counts
	mutex_lock(&mtx);
	out++;
	if (out == 2) {
		in -= 2;
		mid = 0;
		out = 0;
	}
	mutex_unlock(&mtx);
}

int fifo_open(struct inode* inode, struct file* filp)
{
	struct fifo_dev *fifo_devp;

	if (DEBUG > 1) printk(KERN_ALERT "fifo_open()\n");

	fifo_devp = container_of(inode->i_cdev, struct fifo_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = fifo_devp;

	return 0;
}

/*
 * The fifo is implemented using a cirular buffer.
 * It is empty when the read and write pointers are the same.
 * And it is full when the write pointer is just behind the read.
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
	if (DEBUG) printk(KERN_ALERT "  pre-offsets; read, write: %zu, %zu\n",
										read_ptr - fifo_start,
										write_ptr - fifo_start);

	while (left && read_ptr != write_ptr) {

		if (write_ptr > read_ptr) {
			cnt = write_ptr - read_ptr;
		} else {
			cnt = fifo_end - read_ptr;
		}

		pwait(read_mtx);

		if (cnt > left)
			cnt = left;

		pwait(read_mtx);

		if (DEBUG) printk(KERN_ALERT "  read: %zu\n", cnt);

		if (copy_to_user(buf, (void *) read_ptr, cnt) != 0) {
			return -EIO;
		}

		pwait(read_mtx);

		buf += cnt;
		pwait(read_mtx);
		left -= cnt;
		pwait(read_mtx);
		read_ptr += cnt;

		pwait(read_mtx);

		if (read_ptr == fifo_end)
			read_ptr = fifo_start;

		pwait(read_mtx);

		if (DEBUG) printk(KERN_ALERT "  new read offset: %zu\n",
											read_ptr - fifo_start);
	}

	return (count - left);
}

ssize_t fifo_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	size_t cnt;
	size_t left;

	if (DEBUG) printk(KERN_ALERT "fifo_write(%zu)\n", count);
	if (DEBUG) printk(KERN_ALERT "  pre-offsets; read, write: %zu, %zu\n",
										read_ptr - fifo_start,
										write_ptr - fifo_start);

	left = count;

	while (left) {

		if ( (write_ptr == fifo_end) && (read_ptr == fifo_start) ) {
			/* loop end to beginning, full, write_ptr < read_ptr */
			break;
		} else if (write_ptr >= read_ptr) {
			cnt = fifo_end - write_ptr;
		} else { // write_ptr < read_ptr
			cnt = (read_ptr - 1) - write_ptr;
			if (0 == cnt)
				break;
		}

		pwait(write_mtx);

		if (cnt > left)
			cnt = left;

		pwait(write_mtx);

		if (DEBUG) printk(KERN_ALERT "  write: %zu\n", cnt);

		if (copy_from_user((void *) write_ptr, buf, cnt) != 0) {
			return -EIO;
		}

		pwait(write_mtx);

		buf += cnt;
		pwait(write_mtx);
		left -= cnt;
		pwait(write_mtx);
		write_ptr += cnt;
		pwait(write_mtx);

		/* reset to begining if we reach the end */
		if (write_ptr == fifo_end)
			write_ptr = fifo_start;

		pwait(write_mtx);
		if (DEBUG) printk(KERN_ALERT "  new write offset: %zu\n",
										write_ptr - fifo_start);
	}

	return (count - left);
}

int fifo_release(struct inode *inode, struct file *filp)
{
	if (DEBUG > 1) printk(KERN_ALERT "fifo_release()\n");

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
	if (DEBUG > 1) printk(KERN_ALERT "fifo_cleanup()\n");

	if (fifo_major) {
		if (DEBUG > 1) printk(KERN_ALERT "fifo: unregister_chrdev_region()\n");
		unregister_chrdev_region(fifo_major, 1);
	}

	if (fifo_device) {
		if (DEBUG > 1) printk(KERN_ALERT "fifo: device_destroy()\n");
		device_destroy(fifo_class, fifo_major);
	}

	if (cdev_add_done) {
		if (DEBUG > 1) printk(KERN_ALERT "fifo: cdev_del()\n");
		cdev_del(&fifo_devp->cdev);
	}

	if (fifo_devp) {
		if (DEBUG > 1) printk(KERN_ALERT "fifo: kfree()\n");
		kfree(fifo_devp);
	}

	if (fifo_class) {
		if (DEBUG > 1) printk(KERN_ALERT "fifo: class_destroy()\n");
		class_destroy(fifo_class);
	}
}

static int __init fifo_init(void)
{
	int err = 0;

	if (DEBUG > 1) printk(KERN_ALERT "fifo_init()\n");

	read_ptr = &fifo[0];
	write_ptr = &fifo[0];
	fifo_start = &fifo[0];
	fifo_end = &fifo[MAX_DATA-1];

	// pwait()
	in = 0;
	mid = 0;
	out = 0;

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
	if (DEBUG > 1) printk(KERN_ALERT "fifo_exit()\n");

	fifo_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(fifo_init);
module_exit(fifo_exit);

