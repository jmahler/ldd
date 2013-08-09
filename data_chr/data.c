#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "data"

static dev_t data_major;
static int cdev_add_done;
struct class *data_class;
struct device *data_device;

struct data_dev {
	struct cdev cdev;
} *data_devp;

struct file_operations data_fops = {
	.owner = THIS_MODULE,
};

static void data_cleanup(void)
{
	if (data_major) {
		unregister_chrdev_region(data_major, 1);
	}

	if (data_device) {
		device_destroy(data_class, data_major);
	}

	if (cdev_add_done) {
		cdev_del(&data_devp->cdev);
	}

	if (data_devp) {
		kfree(data_devp);
	}

	if (data_class) {
		class_destroy(data_class);
	}
}

static int __init data_init(void)
{
	int err = 0;

	data_major = 0;
	data_class = NULL;
	data_device = NULL;
	data_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&data_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto err_out;
	}

	/* populate sysfs entries */
	/* /sys/class/data/data0/ */
	data_class = class_create(THIS_MODULE, DEVICE_NAME);

	data_devp = kmalloc(sizeof(struct data_dev), GFP_KERNEL);
	if (!data_devp) {
		printk(KERN_WARNING "Unable to kmalloc data_devp\n");
		err = -ENOMEM;
		goto err_out;
	}

	cdev_init(&data_devp->cdev, &data_fops);
	data_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&data_devp->cdev, data_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		goto err_out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/data0 */
	data_device = device_create(data_class, NULL,
							MKDEV(MAJOR(data_major), 0), NULL, "data%d",0);

	return 0;  /* success */

err_out:
	data_cleanup();
	return err;
}

static void __exit data_exit(void)
{
	data_cleanup();
}

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");

module_init(data_init);
module_exit(data_exit);
