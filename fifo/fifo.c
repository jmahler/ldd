
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

// {{{ fifo_init()
static int __init fifo_init(void)
{
	int ret;

	// allocate a device major number
	if (alloc_chrdev_region(&fifo_dev_number, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_ERR "Unable to allocate device major number\n");

		return -1;  // ERROR
	}

	// create a sysfs entry /sys/class/fifo
	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(fifo_class)) {
		printk(KERN_ERR "Unable to create fifo class memory\n");

		unregister_chrdev_region(fifo_dev_number, 0);

		return PTR_ERR(fifo_class);
	}

	// allocate memory
	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		printk(KERN_ERR "Unable to kmalloc memory\n");

		unregister_chrdev_region(fifo_dev_number, 0);
		class_destroy(fifo_class);

		return -ENOMEM;
	}

	// connect the file operations with the cdev
	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;

	// add our cdev to the kernel
	ret = cdev_add(&fifo_devp->cdev, fifo_dev_number, 1);
	if (ret) {
		printk(KERN_ERR "Unable to add cdev\n");

		unregister_chrdev_region(fifo_dev_number, 0);
		class_destroy(fifo_class);
		kfree(fifo_devp);

		return ret;
	}

	// notify udev, so it will create the devices
	device_create(fifo_class, NULL, fifo_dev_number, NULL, "fifo0");

	return 0;  // OK
}
// }}}

// {{{ fifo_exit()
static void __exit fifo_exit(void)
{
	printk(KERN_INFO "%s fifo_exit()\n", DEVICE_NAME);

	// release the device major number
	unregister_chrdev_region(fifo_dev_number, 0);

	// tell udev to destroy the device
	//device_destroy(fifo_class, MKDEV(MAJOR(fifo_dev_number), 0));
	device_destroy(fifo_class, fifo_dev_number);

	// remove our cdev from the kernel
	cdev_del(&fifo_devp->cdev);

	// free memory
	kfree(fifo_devp);

	// remove the sysfs entry /sys/class/fifo
	class_destroy(fifo_class);
}
// }}}

module_init(fifo_init);
module_exit(fifo_exit);

