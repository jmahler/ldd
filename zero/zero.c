
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "zero"

static dev_t zero_major;
struct class *zero_class;
struct device *zero_device;

struct zero_dev {
	struct cdev cdev;
} *zero_devp;

static int zero_open(struct inode *inode, struct file *filp)
{
	struct zero_dev *zero_devp;

	zero_devp = container_of(inode->i_cdev, struct zero_dev, cdev);

	filp->private_data = zero_devp;

	return 0;
}

static ssize_t zero_read(struct file *filp, char __user *buf,
					size_t count, loff_t *f_pos)
{
	if (clear_user((void __user *) buf, count) > 0)
		return -EFAULT;

	return count;
}

ssize_t zero_write(struct file *filp, const char __user *buf,
					size_t count, loff_t *f_pos)
{
	return count;
}

static int zero_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations zero_fops = {
	.owner = THIS_MODULE,
	.open = zero_open,
	.read = zero_read,
	.write = zero_write,
	.release = zero_release,
};

static int __init zero_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&zero_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_warn("Unable to register device\n");
		goto err_chrdev_region;
	}

	zero_class = class_create(THIS_MODULE, DEVICE_NAME);

	zero_devp = kmalloc(sizeof(struct zero_dev), GFP_KERNEL);
	if (!zero_devp) {
		pr_warn("Unable to kmalloc zero_devp\n");
		err = -ENOMEM;
		goto err_malloc_zero_devp;
	}

	cdev_init(&zero_devp->cdev, &zero_fops);
	zero_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&zero_devp->cdev, zero_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		goto err_cdev_add;
	}

	zero_device = device_create(zero_class, NULL,
				MKDEV(MAJOR(zero_major), 0), NULL, "zero%d", 0);
	if (IS_ERR(zero_device)) {
		pr_warn("device_create failed\n");
		err = PTR_ERR(zero_device);
		goto err_device_create;
	}

	return 0;

err_device_create:
	cdev_del(&zero_devp->cdev);
err_cdev_add:
	kfree(zero_devp);
err_malloc_zero_devp:
	class_destroy(zero_class);
	unregister_chrdev_region(zero_major, 1);
err_chrdev_region:

	return err;
}

static void __exit zero_exit(void)
{
	device_destroy(zero_class, zero_major);

	cdev_del(&zero_devp->cdev);

	kfree(zero_devp);

	class_destroy(zero_class);

	unregister_chrdev_region(zero_major, 1);
}

module_init(zero_init);
module_exit(zero_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

