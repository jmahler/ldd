#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "null"

static dev_t null_major;
struct class *null_class;
struct device *null_device;

struct null_dev {
	struct cdev cdev;
} *null_devp;


static int null_open(struct inode* inode, struct file* filp)
{
	struct null_dev *null_devp;

	null_devp = container_of(inode->i_cdev, struct null_dev, cdev);

	filp->private_data = null_devp;

	return 0;
}

static ssize_t null_read(struct file *filp, char __user *buf,
								size_t count, loff_t *f_pos)
{
	return 0;
}

static ssize_t null_write(struct file *filp, const char __user *buf,
								size_t count, loff_t *f_pos)
{
	return count;
}

static int null_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations null_fops = {
	.owner = THIS_MODULE,
	.open = null_open,
	.read = null_read,
	.write = null_write,
	.release = null_release,
};

static int __init null_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&null_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_warn("Unable to register device\n");
		goto err_chrdev_region;
	}

	null_class = class_create(THIS_MODULE, DEVICE_NAME);

	null_devp = kmalloc(sizeof(struct null_dev), GFP_KERNEL);
	if (!null_devp) {
		pr_warn("Unable to kmalloc null_devp\n");
		err = -ENOMEM;
		goto err_malloc_null_devp;
	}

	cdev_init(&null_devp->cdev, &null_fops);
	null_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&null_devp->cdev, null_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		goto err_cdev_add;
	}

	null_device = device_create(null_class, NULL,
							MKDEV(MAJOR(null_major), 0), NULL, "null%d",0);
	if (IS_ERR(null_device)) {
		pr_warn("device_create failed\n");
		err = PTR_ERR(null_device);
		goto err_device_create;
	}

	return 0;

err_device_create:
	cdev_del(&null_devp->cdev);
err_cdev_add:
	kfree(null_devp);
err_malloc_null_devp:
	class_destroy(null_class);
	unregister_chrdev_region(null_major, 1);
err_chrdev_region:

	return err;
}

static void __exit null_exit(void)
{
	device_destroy(null_class, null_major);

	cdev_del(&null_devp->cdev);

	kfree(null_devp);

	class_destroy(null_class);

	unregister_chrdev_region(null_major, 1);
}

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");

module_init(null_init);
module_exit(null_exit);
