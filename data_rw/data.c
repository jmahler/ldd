
#define DEVICE_NAME "data"
#define MAX_DATA 128

#define DEBUG 1

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
	ssize_t cur_ofs;  // current offset (position)
} *data_devp;

struct class *data_class;
struct device *data_device;

int data_open(struct inode* inode, struct file* filp)
{
	struct data_dev *data_devp;

	if (DEBUG) printk(KERN_ALERT "data_open()\n");

	data_devp = container_of(inode->i_cdev, struct data_dev, cdev);
	data_devp->cur_ofs = 0;

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = data_devp;

	return 0;
}

ssize_t data_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	size_t cnt;
	size_t cur_ofs;
	char *datp;
	size_t left;

	cur_ofs = data_devp->cur_ofs;
	datp = data_devp->data;
	left = count;

	if (DEBUG) printk(KERN_ALERT "data_read(%zu)\n", count);

	while (left) {
		/* limit the size of this transfer */
		cnt = MAX_DATA - cur_ofs;
		if (cnt > left)
			cnt = left;

		if (DEBUG) printk(KERN_ALERT "  read: %zu\n", cnt);

		if (copy_to_user(buf, (void *) (datp + cur_ofs), cnt) != 0) {
			return -EIO;
		}

		buf += cnt;
		left -= cnt;
		cur_ofs += cnt;

		/* reset to begining if we reach the end */
		if (cur_ofs == MAX_DATA)
			cur_ofs = 0;
	}

	if (DEBUG) printk(KERN_ALERT "  new offset: %zu\n", cur_ofs);

	data_devp->cur_ofs = cur_ofs;

	return count;
}

ssize_t data_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct data_dev *data_devp = filp->private_data;
	size_t cnt;
	size_t cur_ofs;
	char *datp;
	size_t left;

	if (DEBUG) printk(KERN_ALERT "data_write(%zu)\n", count);

	cur_ofs = data_devp->cur_ofs;
	datp = data_devp->data;
	left = count;

	while (left) {
		/* limit the size of this transfer */
		cnt = MAX_DATA - cur_ofs;
		if (cnt > left)
			cnt = left;

		if (DEBUG) printk(KERN_ALERT "  write: %zu\n", cnt);

		if (copy_from_user((void *) (datp + cur_ofs), buf, cnt) != 0) {
			return -EIO;
		}

		buf += cnt;
		left -= cnt;
		cur_ofs += cnt;

		/* reset to begining if we reach the end */
		if (cur_ofs == MAX_DATA)
			cur_ofs = 0;
	}

	if (DEBUG) printk(KERN_ALERT "  new offset: %zu\n", cur_ofs);

	data_devp->cur_ofs = cur_ofs;

	return count;
}

int data_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "data_release()\n");

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
	if (DEBUG) printk(KERN_ALERT "data_cleanup()\n");

	if (data_major) {
		if (DEBUG) printk(KERN_ALERT "data: unregister_chrdev_region()\n");
		unregister_chrdev_region(data_major, 1);
	}

	if (data_device) {
		if (DEBUG) printk(KERN_ALERT "data: device_destroy()\n");
		device_destroy(data_class, data_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "data: cdev_del()\n");
		cdev_del(&data_devp->cdev);
	}

	if (data_devp) {
		if (DEBUG) printk(KERN_ALERT "data: kfree()\n");
		kfree(data_devp);
	}

	if (data_class) {
		if (DEBUG) printk(KERN_ALERT "data: class_destroy()\n");
		class_destroy(data_class);
	}
}

static int __init data_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "data_init()\n");

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
	/* /sys/class/data/data0/ */
	data_class = class_create(THIS_MODULE, DEVICE_NAME);

	data_devp = kmalloc(sizeof(struct data_dev), GFP_KERNEL);
	if (!data_devp) {
		printk(KERN_WARNING "Unable to kmalloc data_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&data_devp->cdev, &data_fops);
	data_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&data_devp->cdev, data_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/data0 */
	data_device = device_create(data_class, NULL, MKDEV(MAJOR(data_major), 0), NULL, "data%d",0);

	return 0;  /* success */

out:
	data_cleanup();
	return err;
}

static void __exit data_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "data_exit()\n");

	data_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(data_init);
module_exit(data_exit);

