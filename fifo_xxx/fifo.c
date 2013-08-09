
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

static dev_t fifo_major;
static int cdev_add_done;
struct class *fifo_class;
struct device *fifo_device;

struct fifo_dev {
	struct cdev cdev;
	char fifo[MAX_DATA];
	char *read_ptr;
	char *write_ptr;
	char *fifo_start;
	char *fifo_end;
	int empty;
} *fifo_devp;

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

	fifo_devp = container_of(inode->i_cdev, struct fifo_dev, cdev);

	filp->private_data = fifo_devp;

	return 0;
}

ssize_t fifo_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	size_t left;

	struct fifo_dev *dev = filp->private_data;

	left = count;

	while (left) {

		if (dev->empty) {
			break;
		}

		pwait(read_mtx);

		if (copy_to_user(buf, (void *) dev->read_ptr, 1) != 0) {
			return -EIO;
		}
		left--;

		pwait(read_mtx);

		if (dev->read_ptr == dev->fifo_end) {
			dev->read_ptr = dev->fifo_start;
			pwait(read_mtx);
		} else {
			(dev->read_ptr)++;
			pwait(read_mtx);
		}

		pwait(read_mtx);
		if (dev->read_ptr == dev->write_ptr) {
			dev->empty = 1;
		}
	}

	return (count - left);
}

ssize_t fifo_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	size_t left;

	struct fifo_dev *dev = filp->private_data;

	left = count;

	while (left) {

		if (!(dev->empty) && (dev->read_ptr == dev->write_ptr)) {
			break;
		}

		if (copy_from_user((void *) dev->write_ptr, buf, 1) != 0) {
			return -EIO;
		}
		left--;

		if (dev->empty)
			dev->empty = 0;

		pwait(write_mtx);

		if (dev->write_ptr == dev->fifo_end) {
			dev->write_ptr = dev->fifo_start;
			pwait(write_mtx);
		} else {
			(dev->write_ptr)++;
			pwait(write_mtx);
		}
	}

	return (count - left);
}

int fifo_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations fifo_fops = {
	.owner = THIS_MODULE,
	.open = fifo_open,
	.read = fifo_read,
	.write = fifo_write,
	.release = fifo_release,
};


static ssize_t read_offset_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	struct fifo_dev *fifo_devp = dev_get_drvdata(dev);
	return sprintf(buf, "%lu\n", fifo_devp->read_ptr - fifo_devp->fifo_start);
}

static ssize_t read_offset_store(struct device *dev,
									struct device_attribute *attr,
									const char *buf,
									size_t count)
{
	return 0;  // stored nothing
}

static DEVICE_ATTR(read_offset, 0666, read_offset_show, read_offset_store);

static ssize_t write_offset_show(struct device *dev,
								struct device_attribute *attr,
								char *buf)
{
	struct fifo_dev *fifo_devp = dev_get_drvdata(dev);
	return sprintf(buf, "%lu\n", fifo_devp->write_ptr - fifo_devp->fifo_start);
}

static ssize_t write_offset_store(struct device *dev,
									struct device_attribute *attr,
									const char *buf,
									size_t count)
{
	return 0;
}

static DEVICE_ATTR(write_offset, 0666, write_offset_show, write_offset_store);


static void fifo_cleanup(void)
{
	if (fifo_major) {
		unregister_chrdev_region(fifo_major, 1);
	}

	if (fifo_device) {
		device_destroy(fifo_class, fifo_major);
	}

	if (cdev_add_done) {
		cdev_del(&fifo_devp->cdev);
	}

	if (fifo_devp) {
		kfree(fifo_devp);
	}

	if (fifo_class) {
		class_destroy(fifo_class);
	}
}


static int __init fifo_init(void)
{
	int err = 0;

	/* defaults, tested by cleanup() */
	fifo_major = 0;
	fifo_class = NULL;
	fifo_device = NULL;
	fifo_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&fifo_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto err_out;
	}

	/* populate sysfs entries */
	/* /sys/class/fifo/fifo0/ */
	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);

	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		printk(KERN_WARNING "Unable to kmalloc fifo_devp\n");
		err = -ENOMEM;
		goto err_out;
	}

	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;

	fifo_devp->fifo_start = &fifo_devp->fifo[0];
	fifo_devp->fifo_end = &fifo_devp->fifo[MAX_DATA-1];
	fifo_devp->read_ptr = &fifo_devp->fifo[0];
	fifo_devp->write_ptr = &fifo_devp->fifo[0];
	fifo_devp->empty = 1;

	err = cdev_add(&fifo_devp->cdev, fifo_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		goto err_out;
	}
	cdev_add_done = 1;

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/fifo0 */
	fifo_device = device_create(fifo_class, NULL, MKDEV(MAJOR(fifo_major), 0), fifo_devp, "fifo%d",0);

	err = device_create_file(fifo_device, &dev_attr_read_offset);
	if (err) {
		goto err_out;
	}

	err = device_create_file(fifo_device, &dev_attr_write_offset);
	if (err) {
		goto err_out;
	}


	return 0;  /* success */

err_out:
	fifo_cleanup();
	return err;
}

static void __exit fifo_exit(void)
{
	fifo_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(fifo_init);
module_exit(fifo_exit);
