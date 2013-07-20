
#define DEVICE_NAME "null"

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t null_major;
static int cdev_add_done;

struct null_dev {
	struct cdev cdev;
} *null_devp;

struct class *null_class;
struct device *null_device;

int null_open(struct inode* inode, struct file* filp)
{
	struct null_dev *null_devp;

	if (DEBUG) printk(KERN_ALERT "null_open()\n");

	null_devp = container_of(inode->i_cdev, struct null_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = null_devp;

	return 0;
}

ssize_t null_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	if (DEBUG) printk(KERN_ALERT "null_read(%zu)\n", count);

	return 0;  /* EOF */
}

ssize_t null_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	if (DEBUG) printk(KERN_ALERT "null_write(%zu)\n", count);

	return count;
}

int null_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "null_release()\n");

	return 0;
}

struct file_operations null_fops = {
	.owner = THIS_MODULE,
	.open = null_open,
	.read = null_read,
	.write = null_write,
	.release = null_release,
};

static void null_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "null_cleanup()\n");

	if (null_major) {
		if (DEBUG) printk(KERN_ALERT "null: unregister_chrdev_region()\n");
		unregister_chrdev_region(null_major, 1);
	}

	if (null_device) {
		if (DEBUG) printk(KERN_ALERT "null: device_destroy()\n");
		device_destroy(null_class, null_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "null: cdev_del()\n");
		cdev_del(&null_devp->cdev);
	}

	if (null_devp) {
		if (DEBUG) printk(KERN_ALERT "null: kfree()\n");
		kfree(null_devp);
	}

	if (null_class) {
		if (DEBUG) printk(KERN_ALERT "null: class_destroy()\n");
		class_destroy(null_class);
	}
}

static int __init null_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "null_init()\n");

	/* defaults, tested by cleanup() */
	null_major = 0;
	null_class = NULL;
	null_device = NULL;
	null_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&null_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/null/null0/ */
	null_class = class_create(THIS_MODULE, DEVICE_NAME);

	null_devp = kmalloc(sizeof(struct null_dev), GFP_KERNEL);
	if (!null_devp) {
		printk(KERN_WARNING "Unable to kmalloc null_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&null_devp->cdev, &null_fops);
	null_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&null_devp->cdev, null_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/null0 */
	null_device = device_create(null_class, NULL, MKDEV(MAJOR(null_major), 0), NULL, "null%d",0);

	return 0;  /* success */

out:
	null_cleanup();
	return err;
}

static void __exit null_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "null_exit()\n");

	null_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(null_init);
module_exit(null_exit);

