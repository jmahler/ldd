
#define DEVICE_NAME "datchr"

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t datchr_major;
static int cdev_add_done;

struct datchr_dev {
	struct cdev cdev;
} *datchr_devp;

struct class *datchr_class;
struct device *datchr_device;

struct file_operations datchr_fops = {
	.owner	 = THIS_MODULE,
};

static void datchr_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "datchr_cleanup()\n");

	if (datchr_major) {
		if (DEBUG) printk(KERN_ALERT "datchr: unregister_chrdev_region()\n");
		unregister_chrdev_region(datchr_major, 1);
	}

	if (datchr_device) {
		if (DEBUG) printk(KERN_ALERT "datchr: device_destroy()\n");
		device_destroy(datchr_class, datchr_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "datchr: cdev_del()\n");
		cdev_del(&datchr_devp->cdev);
	}

	if (datchr_devp) {
		if (DEBUG) printk(KERN_ALERT "datchr: kfree()\n");
		kfree(datchr_devp);
	}

	if (datchr_class) {
		if (DEBUG) printk(KERN_ALERT "datchr: class_destroy()\n");
		class_destroy(datchr_class);
	}
}

static int __init datchr_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "datchr_init()\n");

	/* defaults, tested by cleanup() */
	datchr_major = 0;
	datchr_class = NULL;
	datchr_device = NULL;
	datchr_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&datchr_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/datchr/datchr0/ */
	datchr_class = class_create(THIS_MODULE, DEVICE_NAME);

	datchr_devp = kmalloc(sizeof(struct datchr_dev), GFP_KERNEL);
	if (!datchr_devp) {
		printk(KERN_WARNING "Unable to kmalloc datchr_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&datchr_devp->cdev, &datchr_fops);
	datchr_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&datchr_devp->cdev, datchr_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/datchr0 */
	datchr_device = device_create(datchr_class, NULL, MKDEV(MAJOR(datchr_major), 0), NULL, "datchr%d",0);

	return 0;  /* success */

out:
	datchr_cleanup();
	return err;
}

static void __exit datchr_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "datchr_exit()\n");

	datchr_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(datchr_init);
module_exit(datchr_exit);

