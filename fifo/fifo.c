
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "fifo"

// per device structure
struct fifo_dev {
	struct cdev cdev;
	char buf[10];
} *fifo_devp;

static dev_t fifo_dev_number;
struct class *fifo_class;

static struct file_operations fifo_fops = {
	.owner    = THIS_MODULE,
//	.open     = fifo_open,
//	.release  = fifo_release,
//	.read     = fifo_read,
//	.write    = fifo_write,
//	.ioctl    = fifo_ioctl,
};

static int __init fifo_init(void)
{
	int ret;

	// allocate a device major number
	if (alloc_chrdev_region(&fifo_dev_number, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_ERR "Unable to allocate device major number");

		return -1;  // ERROR
	}
	printk(KERN_INFO "%s: alloc_chrdev_region() (MAJOR, MINOR) -> (%d, %d)", DEVICE_NAME, MAJOR(fifo_dev_number), MINOR(fifo_dev_number));

	printk(KERN_INFO "%s: class_create()", DEVICE_NAME);
	// create a sysfs entry /sys/class/fifo
	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);

	printk(KERN_INFO "%s: kmalloc()", DEVICE_NAME);
	// allocate memory
	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		printk(KERN_ERR "Unable to kmalloc memory");
		return -ENOMEM;
	}

	// connect the file operations with the cdev
	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;

	// add our cdev to the kernel
	ret = cdev_add(&fifo_devp->cdev, fifo_dev_number, 1);
	if (ret) {
		printk("Unable to add cdev");
		return ret;
	}

	/*
	// notify udev, so it will create the devices
	device_create(fifo_class, NULL, MKDEV(MAJOR(fifo_dev_number), 0),
			"fifo%d", 0);
	*/

	printk(KERN_INFO "%s loaded", DEVICE_NAME);

	return 0;  // OK
}

static void __exit fifo_exit(void)
{
	printk(KERN_INFO "%s fifo_exit()", DEVICE_NAME);


	unregister_chrdev_region(fifo_dev_number, 0);

	// TODO device_destroy

	cdev_del(&fifo_devp->cdev);

	// free memory
	kfree(fifo_devp);

	// remove the sysfs entry /sys/class/fifo
	class_destroy(fifo_class);
}

module_init(fifo_init);
module_exit(fifo_exit);

