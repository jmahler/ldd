
#define DEVICE_NAME "data"
#define MAX_DATA 128

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t data_major;
static int cdev_add_done;

struct data_dev {
	struct cdev cdev;
	char data[MAX_DATA];
} *data_devp;

struct class *data_class;
struct device *data_device;

int data_open(struct inode* inode, struct file* filp)
{
	struct data_dev *data_devp;

	data_devp = container_of(inode->i_cdev, struct data_dev, cdev);

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = data_devp;

	return 0;
}

ssize_t data_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	size_t cnt;

	cnt = (count > MAX_DATA) ? MAX_DATA : count;

	if (copy_to_user(buf, (void *) data_devp->data, cnt) != 0) {
		return -EIO;
	}

	return cnt;
}

ssize_t data_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	size_t cnt;

	cnt = (count > MAX_DATA) ? MAX_DATA : count;

	if (copy_from_user((void *) data_devp->data, buf, cnt) != 0) {
		return -EIO;
	}

	return cnt;
}

int data_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations data_fops = {
	.owner = THIS_MODULE,
	.open = data_open,
	.read = data_read,
	.write = data_write,
	.release = data_release,
};

static void data_cleanup(void)
{
	if (!data_device) {
		device_destroy(data_class, data_major);
		data_device = NULL;
	}

	if (cdev_add_done) {
		cdev_del(&data_devp->cdev);
		cdev_add_done = 0;
	}

	if (!data_devp) {
		kfree(data_devp);
		data_devp = NULL;
	}

	if (!data_class) {
		class_destroy(data_class);
		data_class = NULL;
	}

	if (data_major) {
		unregister_chrdev_region(data_major, 1);
		data_major = 0;
	}
}

static int __init data_init(void)
{
	int err = 0;

	/* defaults, tested by cleanup() */
	data_major = 0;
	data_class = NULL;
	data_device = NULL;
	data_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&data_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	data_class = class_create(THIS_MODULE, DEVICE_NAME);

	data_devp = kmalloc(sizeof(struct data_dev), GFP_KERNEL);
	if (!data_devp) {
		printk(KERN_WARNING "Unable to kmalloc data_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&data_devp->cdev, &data_fops);
	data_devp->cdev.owner = THIS_MODULE;
	data_devp->cdev.ops = &data_fops;
	err = cdev_add(&data_devp->cdev, data_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 0;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	data_device = device_create(data_class, NULL, data_major, NULL, "data%d",0);

	return 0;  /* success */

out:
	data_cleanup();
	return err;
}

static void __exit data_exit(void)
{
	data_cleanup();
}

module_init(data_init);
module_exit(data_exit);

MODULE_LICENSE("GPL");
