
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "data"
#define MAX_DATA 128

static dev_t data_major;
struct class *data_class;
struct device *data_device;

struct data_dev {
	struct cdev cdev;
	char data[MAX_DATA];
	loff_t cur_ofs;  /* current offset */
} *data_devp;

static int data_open(struct inode *inode, struct file *filp)
{
	struct data_dev *data_devp;

	data_devp = container_of(inode->i_cdev, struct data_dev, cdev);
	data_devp->cur_ofs = 0;

	filp->private_data = data_devp;

	return 0;
}

static ssize_t data_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	loff_t cur_ofs;
	char *datp;
	size_t left;

	cur_ofs = data_devp->cur_ofs;
	datp = data_devp->data;
	left = MAX_DATA - cur_ofs;

	count = (count > left) ? left : count;

	if (copy_to_user(buf, (void *) (datp + cur_ofs), count) != 0)
		return -EIO;

	data_devp->cur_ofs = cur_ofs + count;

	return count;
}

static ssize_t data_write(struct file *filp, const char __user *buf,
						size_t count, loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	loff_t cur_ofs;
	char *datp;
	size_t left;

	cur_ofs = data_devp->cur_ofs;
	datp = data_devp->data;
	left = MAX_DATA - cur_ofs;

	count = (count > left) ? left : count;

	if (copy_from_user((void *) (datp + cur_ofs), buf, count) != 0)
		return -EIO;

	data_devp->cur_ofs = cur_ofs + count;

	return count;
}

static int data_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations data_fops = {
	.owner = THIS_MODULE,
	.open = data_open,
	.read = data_read,
	.write = data_write,
	.release = data_release,
};

static int __init data_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&data_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_warn("Unable to register device\n");
		goto err_chrdev_region;
	}

	data_class = class_create(THIS_MODULE, DEVICE_NAME);

	data_devp = kmalloc(sizeof(struct data_dev), GFP_KERNEL);
	if (!data_devp) {
		pr_warn("Unable to kmalloc data_devp\n");
		err = -ENOMEM;
		goto err_malloc_devp;
	}

	cdev_init(&data_devp->cdev, &data_fops);
	data_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&data_devp->cdev, data_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		goto err_cdev_add;
	}

	data_device = device_create(data_class, NULL,
			MKDEV(MAJOR(data_major), 0), NULL, "data%d", 0);
	if (IS_ERR(data_device)) {
		pr_warn("device_create failed\n");
		err = PTR_ERR(data_device);
		goto err_device_create;
	}

	return 0;  /* success */

err_device_create:
	cdev_del(&data_devp->cdev);
err_cdev_add:
	kfree(data_devp);
err_malloc_devp:
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

module_init(data_init);
module_exit(data_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

