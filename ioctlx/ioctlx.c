
#define DEVICE_NAME "ioctlx"

#define DEBUG 1

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t ioctlx_major;
static int cdev_add_done;

struct ioctlx_dev {
	struct cdev cdev;
} *ioctlx_devp;

struct class *ioctlx_class;
struct device *ioctlx_device;

int x;

int ioctlx_open(struct inode *inode, struct file *filp)
{
	struct ioctlx_dev *ioctlx_devp;

	if (DEBUG) printk(KERN_ALERT "ioctlx_open()\n");

	ioctlx_devp = container_of(inode->i_cdev, struct ioctlx_dev, cdev);

	filp->private_data = ioctlx_devp;

	return 0;
}

#define IOCTLX_IOC_MAGIC 'm'

#define IOCTLX_IOCRESET _IO(IOCTLX_IOC_MAGIC,  1)
#define IOCTLX_IOCWX	_IOW(IOCTLX_IOC_MAGIC, 2, int)
#define IOCTLX_IOCRX	_IOR(IOCTLX_IOC_MAGIC, 3, int)

#define IOCTLX_IOC_MAXNR 5

static long ioctlx_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	int err = 0;
	int retval = 0;

	//struct ioctlx_dev *ioctlx_dev = filp->private_data;

	if (DEBUG) printk(KERN_ALERT "ioctlx_ioctl(cmd=%d, arg=%lu)\n", cmd, arg);

	if (_IOC_TYPE(cmd) != IOCTLX_IOC_MAGIC) {
		printk(KERN_ALERT "invalid ioctl magic\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IOCTLX_IOC_MAXNR) {
		printk(KERN_ALERT "ioctl beyond maximum\n");
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch (cmd) {
		case IOCTLX_IOCRESET:
			/* takes no argument, sets values to default */
			if (DEBUG) printk(KERN_ALERT "  IOCTLX_IOCRESET\n");
			x = 0;
			break;
		case IOCTLX_IOCRX:
			/* read integer */
			if (DEBUG) printk(KERN_ALERT "  IOCTLX_IOCRX\n");
			retval = __put_user(x, (int __user *) arg);
			break;
		case IOCTLX_IOCWX:
			/* write integer */
			if (DEBUG) printk(KERN_ALERT "  IOCTLX_IOCWX\n");
			retval = __get_user(x, (int __user *) arg);
			break;
		default:
			return -ENOTTY;  /* POSIX standard */
			//return -EINVAL;  /* common */
			/* Pg. 161 Linux Device Drivers (2005) */
	}

	if (DEBUG) printk(KERN_ALERT "  return %d\n", retval);

	return retval;
}

int ioctlx_release(struct inode *inode, struct file *filp)
{
	if (DEBUG) printk(KERN_ALERT "ioctlx_release()\n");

	return 0;
}

struct file_operations ioctlx_fops = {
	.owner = THIS_MODULE,
	.open = ioctlx_open,
	.unlocked_ioctl = ioctlx_ioctl,
	.release = ioctlx_release,
};

static void ioctlx_cleanup(void)
{
	if (DEBUG) printk(KERN_ALERT "ioctlx_cleanup()\n");

	if (ioctlx_major) {
		if (DEBUG) printk(KERN_ALERT "ioctlx: unregister_chrdev_region()\n");
		unregister_chrdev_region(ioctlx_major, 1);
	}

	if (ioctlx_device) {
		if (DEBUG) printk(KERN_ALERT "ioctlx: device_destroy()\n");
		device_destroy(ioctlx_class, ioctlx_major);
	}

	if (cdev_add_done) {
		if (DEBUG) printk(KERN_ALERT "ioctlx: cdev_del()\n");
		cdev_del(&ioctlx_devp->cdev);
	}

	if (ioctlx_devp) {
		if (DEBUG) printk(KERN_ALERT "ioctlx: kfree()\n");
		kfree(ioctlx_devp);
	}

	if (ioctlx_class) {
		if (DEBUG) printk(KERN_ALERT "ioctlx: class_destroy()\n");
		class_destroy(ioctlx_class);
	}
}

static int __init ioctlx_init(void)
{
	int err = 0;

	x = 0;

	if (DEBUG) printk(KERN_ALERT "ioctlx_init()\n");

	/* defaults, tested by cleanup() */
	ioctlx_major = 0;
	ioctlx_class = NULL;
	ioctlx_device = NULL;
	ioctlx_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&ioctlx_major, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_WARNING "Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/ioctlx/ioctlx0/ */
	ioctlx_class = class_create(THIS_MODULE, DEVICE_NAME);

	ioctlx_devp = kmalloc(sizeof(struct ioctlx_dev), GFP_KERNEL);
	if (!ioctlx_devp) {
		printk(KERN_WARNING "Unable to kmalloc ioctlx_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&ioctlx_devp->cdev, &ioctlx_fops);
	ioctlx_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&ioctlx_devp->cdev, ioctlx_major, 1);
	if (err) {
		printk(KERN_WARNING "cdev_add failed\n");
		//err = err;
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/ioctlx0 */
	ioctlx_device = device_create(ioctlx_class, NULL, MKDEV(MAJOR(ioctlx_major), 0), NULL, "ioctlx%d",0);

	return 0;  /* success */

out:
	ioctlx_cleanup();
	return err;
}

static void __exit ioctlx_exit(void)
{
	if (DEBUG) printk(KERN_ALERT "ioctlx_exit()\n");

	ioctlx_cleanup();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

module_init(ioctlx_init);
module_exit(ioctlx_exit);

