
#define DEVICE_NAME "datsk"
#define MAX_DATA 128

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t datsk_major;
static int cdev_add_done;

struct datsk_dev {
	struct cdev cdev;
	char data[MAX_DATA];
	ssize_t cur_ofs;  // current offset (position)
} *datsk_devp;

struct class *datsk_class;
struct device *datsk_device;

int datsk_open(struct inode* inode, struct file* filp)
{
	struct datsk_dev *datsk_devp;

	if (DEBUG) printk(KERN_ALERT "datsk_open()\n");

	datsk_devp = container_of(inode->i_cdev, struct datsk_dev, cdev);
	datsk_devp->cur_ofs = 0;

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = datsk_devp;

	return 0;
}

ssize_t datsk_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct datsk_dev *datsk_devp = filp->private_data;
	size_t cnt;
	size_t cur_ofs;
	char *datp;
	size_t sent = 0;

	cur_ofs = datsk_devp->cur_ofs;
	datp = datsk_devp->data;

	if (DEBUG) printk(KERN_ALERT "datsk_read(%zu)\n", count);

	while (sent != count) {
		if (cur_ofs == MAX_DATA)
			cur_ofs = 0;

		cnt = cur_ofs + count;
		if (cnt > MAX_DATA)
			cnt = MAX_DATA;

		if (DEBUG) printk(KERN_ALERT "  read: %zu\n", cnt);

		if (copy_to_user(buf, (void *) (datp + cur_ofs), cnt) != 0) {
			return -EIO;
		}
		
		buf += cnt;
		cur_ofs += cnt;
		sent += cnt;
	}

	if (DEBUG) printk(KERN_ALERT "  new offset: %zu\n", cur_ofs);

	datsk_devp->cur_ofs = cur_ofs;

	return count;
}

ssize_t datsk_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct datsk_dev *datsk_devp = filp->private_data;
	size_t cnt;
	size_t cur_ofs;
	char *datp;
	size_t sent = 0;

	if (DEBUG) printk(KERN_ALERT "datsk_write(%zu)\n", count);

	cur_ofs = datsk_devp->cur_ofs;
	datp = datsk_devp->data;

	while (sent != count) {
		if (cur_ofs == MAX_DATA)
			cur_ofs = 0;

		cnt = cur_ofs + count;
		if (cnt > MAX_DATA)
			cnt = MAX_DATA;

		if (DEBUG) printk(KERN_ALERT "  write: %zu\n", cnt);

		if (copy_from_user((void *) (datp + cur_ofs), buf, cnt) != 0) {
			return -EIO;
		}

		buf += cnt;
		cur_ofs += cnt;
		sent += cnt;
	}

	if (DEBUG) printk(KERN_ALERT "  new offset: %zu\n", cur_ofs);

	datsk_devp->cur_ofs = cur_ofs;

	return count;
}

static loff_t datsk_llseek(struct file *filp, loff_t offset, int orig)
{
	struct datsk_dev *datsk_devp = filp->private_data;
	size_t cur_ofs;

	cur_ofs = datsk_devp->cur_ofs;

	if (DEBUG) printk(KERN_ALERT "datsk_llseek(%zu)\n", cur_ofs);

	switch (orig) {
		case 0: /* SEEK_SET */
			cur_ofs = offset;
			break;
		case 1: /* SEEK_CUR */
			cur_ofs += offset;
		case 2: /* SEEK_END */
			cur_ofs = (MAX_DATA - 1) + offset;
		default:
			return -EINVAL;
	}

	if (cur_ofs < 0 || cur_ofs >= MAX_DATA)
		return -EINVAL;

	if (DEBUG) printk(KERN_ALERT "  new offset: %zu\n", cur_ofs);

	datsk_devp->cur_ofs = cur_ofs;

	return cur_ofs;
}

int datsk_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "datsk_release()\n");

	return 0;
}

struct file_operations datsk_fops = {
	.owner	 = THIS_MODULE,
	.open	 = datsk_open,
	.read	 = datsk_read,
	.write	 = datsk_write,
	.llseek  = datsk_llseek,
	.release = datsk_release,
};

static void datsk_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "datsk_cleanup()\n");

	if (datsk_major) {
		if (DEBUG) printk(KERN_ALERT "datsk: unregister_chrdev_region()\n");
		unregister_chrdev_region(datsk_major, 1);
		datsk_major = 0;
	}

	if (datsk_device) {
		if (DEBUG) printk(KERN_ALERT "datsk: device_destroy()\n");
		device_destroy(datsk_class, datsk_major);
		datsk_device = NULL;
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "datsk: cdev_del()\n");
		cdev_del(&datsk_devp->cdev);
		cdev_add_done = 0;
	}

	if (datsk_devp) {
		if (DEBUG) printk(KERN_ALERT "datsk: kfree()\n");
		kfree(datsk_devp);
		datsk_devp = NULL;
	}

	if (datsk_class) {
		if (DEBUG) printk(KERN_ALERT "datsk: class_destroy()\n");
		class_destroy(datsk_class);
		datsk_class = NULL;
	}
}

static int __init datsk_init(void)
{
	int err = 0;

	if (DEBUG) printk(KERN_ALERT "datsk_init()\n");

	/* defaults, tested by cleanup() */
	datsk_major = 0;
	datsk_class = NULL;
	datsk_device = NULL;
	datsk_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&datsk_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/datsk/datsk0/ */
	datsk_class = class_create(THIS_MODULE, DEVICE_NAME);

	datsk_devp = kmalloc(sizeof(struct datsk_dev), GFP_KERNEL);
	if (!datsk_devp) {
		printk(KERN_WARNING "Unable to kmalloc datsk_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&datsk_devp->cdev, &datsk_fops);
	datsk_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&datsk_devp->cdev, datsk_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/datsk0 */
	datsk_device = device_create(datsk_class, NULL, MKDEV(MAJOR(datsk_major), 0), NULL, "datsk%d",0);

	return 0;  /* success */

out:
	datsk_cleanup();
	return err;
}

static void __exit datsk_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "datsk_exit()\n");

	datsk_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(datsk_init);
module_exit(datsk_exit);
