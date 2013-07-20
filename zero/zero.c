
#define DEVICE_NAME "zero"

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>

static dev_t zero_major;
static int cdev_add_done;

struct zero_dev {
	struct cdev cdev;
} *zero_devp;

struct class *zero_class;
struct device *zero_device;

int zero_open(struct inode* inode, struct file* filp)
{
	struct zero_dev *zero_devp;

	if (DEBUG) printk(KERN_ALERT "zero_open()\n");

	zero_devp = container_of(inode->i_cdev, struct zero_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = zero_devp;

	return 0;
}

ssize_t zero_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	if (DEBUG) printk(KERN_ALERT "zero_read(%zu)\n", count);
	
	if (clear_user((void __user *) buf, count) > 0) {
		return -EFAULT;
	}

	return count;
}

ssize_t zero_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	if (DEBUG) printk(KERN_ALERT "zero_write(%zu)\n", count);

	return count;
}

int zero_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "zero_release()\n");

	return 0;
}

struct file_operations zero_fops = {
	.owner = THIS_MODULE,
	.open = zero_open,
	.read = zero_read,
	.write = zero_write,
	.release = zero_release,
};

static void zero_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "zero_cleanup()\n");

	if (zero_major) {
		if (DEBUG) printk(KERN_ALERT "zero: unregister_chrdev_region()\n");
		unregister_chrdev_region(zero_major, 1);
	}

	if (zero_device) {
		if (DEBUG) printk(KERN_ALERT "zero: device_destroy()\n");
		device_destroy(zero_class, zero_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "zero: cdev_del()\n");
		cdev_del(&zero_devp->cdev);
	}

	if (zero_devp) {
		if (DEBUG) printk(KERN_ALERT "zero: kfree()\n");
		kfree(zero_devp);
	}

	if (zero_class) {
		if (DEBUG) printk(KERN_ALERT "zero: class_destroy()\n");
		class_destroy(zero_class);
	}
}

static int __init zero_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "zero_init()\n");

	/* defaults, tested by cleanup() */
	zero_major = 0;
	zero_class = NULL;
	zero_device = NULL;
	zero_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&zero_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/zero/zero0/ */
	zero_class = class_create(THIS_MODULE, DEVICE_NAME);

	zero_devp = kmalloc(sizeof(struct zero_dev), GFP_KERNEL);
	if (!zero_devp) {
		printk(KERN_WARNING "Unable to kmalloc zero_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&zero_devp->cdev, &zero_fops);
	zero_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&zero_devp->cdev, zero_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/zero0 */
	zero_device = device_create(zero_class, NULL, MKDEV(MAJOR(zero_major), 0), NULL, "zero%d",0);

	return 0;  /* success */

out:
	zero_cleanup();
	return err;
}

static void __exit zero_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "zero_exit()\n");

	zero_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(zero_init);
module_exit(zero_exit);

