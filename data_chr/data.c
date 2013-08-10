#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "data"

static dev_t data_major;
struct class *data_class;
struct device *data_device;

struct data_dev {
	struct cdev cdev;
} *data_devp;

struct file_operations data_fops = {
	.owner = THIS_MODULE,
};

static int __init data_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&data_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		goto err_chrdev_region;
	}

	data_class = class_create(THIS_MODULE, DEVICE_NAME);

	data_devp = kmalloc(sizeof(struct data_dev), GFP_KERNEL);
	if (!data_devp) {
		printk(KERN_WARNING "Unable to kmalloc data_devp\n");
		err = -ENOMEM;
		goto err_malloc_data_devp;
	}

	cdev_init(&data_devp->cdev, &data_fops);
	data_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&data_devp->cdev, data_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		goto err_cdev_add;
	}

	data_device = device_create(data_class, NULL,
							MKDEV(MAJOR(data_major), 0), NULL, "data%d",0);
	if (IS_ERR(data_device)) {
		printk(KERN_WARNING "device_create failed\n");
		err = PTR_ERR(data_device);
		goto err_device_create;
	}

	return 0;  /* success */

err_device_create:
	cdev_del(&data_devp->cdev);
err_cdev_add:
	kfree(data_devp);
err_malloc_data_devp:
	class_destroy(data_class);
	unregister_chrdev_region(data_major, 1);
err_chrdev_region:

	return err;
}

static void __exit data_exit(void)
{
	device_destroy(data_class, data_major);

	cdev_del(&data_devp->cdev);

	kfree(data_devp);

	class_destroy(data_class);

	unregister_chrdev_region(data_major, 1);
}

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");

module_init(data_init);
module_exit(data_exit);
