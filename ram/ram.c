
#define DEVICE_NAME "ram"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static dev_t ram_major;
static int cdev_add_done;

struct ram_dev {
	struct cdev cdev;
	char *ram_ptr;
} *ram_devp;

struct class *ram_class;
struct device *ram_device;

int ram_open(struct inode *inode, struct file *filp)
{
	struct ram_dev *ram_devp;

	pr_debug("ram_open()\n");

	ram_devp = container_of(inode->i_cdev, struct ram_dev, cdev);
	ram_devp->ram_ptr = 0x0;

	/* create access to devp from filp, filp is used in other operations */
	filp->private_data = ram_devp;

	return 0;
}

ssize_t ram_read(struct file *filp, char __user *buf, size_t count,
					loff_t *f_pos)
{
	struct ram_dev *ram_devp;
	char *ram_ptr;

	ram_devp = filp->private_data;
	ram_ptr = ram_devp->ram_ptr;

	pr_debug("ram_read(%p, %zu)\n", ram_ptr, count);

	/* Notice that copy_to_user() is not being used
	 * since it checks if that process is allowed to
	 * access a particular memory range.
	 */
	memcpy((char __user *) buf, ram_ptr, count);

	ram_devp->ram_ptr = (ram_ptr + count);

	return count;
}

ssize_t ram_write(struct file *filp, const char __user *buf, size_t count,
					loff_t *f_pos)
{
	pr_debug("ram_write(), cannot write\n");

	return -EIO;
}

static loff_t ram_llseek(struct file *filp, loff_t offset, int orig)
{
	struct ram_dev *ram_devp = filp->private_data;
	char *ram_ptr;

	ram_ptr = ram_devp->ram_ptr;

	pr_debug("ram_llseek(%p\n)", ram_ptr);

	switch (orig) {
	case 0:  /* SEEK_SET */
		ram_ptr = (char *) offset;
		break;
	case 1: /* SEEK_CUR */
		ram_ptr += offset;
		break;
	case 2: /* SEEK_END */
		/* TODO, end of ram? */
		return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	pr_debug("  new pointer: %p\n", ram_ptr);

	ram_devp->ram_ptr = ram_ptr;

	return (loff_t) ram_ptr;
}

int ram_release(struct inode *inode, struct file *filp)
{
	pr_debug("ram_release()\n");

	return 0;
}

static const struct file_operations ram_fops = {
	.owner = THIS_MODULE,
	.open = ram_open,
	.read = ram_read,
	.write = ram_write,
	.llseek = ram_llseek,
	.release = ram_release,
};

static void ram_cleanup(void)
{
	pr_debug("ram_cleanup()\n");

	if (ram_major) {
		pr_debug("ram: unregister_chrdev_region()\n");
		unregister_chrdev_region(ram_major, 1);
	}

	if (ram_device) {
		pr_debug("ram: device_destroy()\n");
		device_destroy(ram_class, ram_major);
	}

	if (cdev_add_done) {
		pr_debug("ram: cdev_del()\n");
		cdev_del(&ram_devp->cdev);
	}

	if (ram_devp) {
		pr_debug("ram: kfree()\n");
		kfree(ram_devp);
	}

	if (ram_class) {
		pr_debug("ram: class_destroy()\n");
		class_destroy(ram_class);
	}
}

static int __init ram_init(void)
{
	int err = 0;

	pr_debug("ram_init()\n");

	/* defaults, tested by cleanup() */
	ram_major = 0;
	ram_class = NULL;
	ram_device = NULL;
	ram_devp = NULL;
	cdev_add_done = 0;

	if (alloc_chrdev_region(&ram_major, 0, 1, DEVICE_NAME) < 0) {
		pr_warn("Unable to register device\n");
		err = -1;
		goto out;
	}

	/* populate sysfs entries */
	/* /sys/class/ram/ram0/ */
	ram_class = class_create(THIS_MODULE, DEVICE_NAME);

	ram_devp = kmalloc(sizeof(struct ram_dev), GFP_KERNEL);
	if (!ram_devp) {
		pr_warn("Unable to kmalloc ram_devp\n");
		err = -ENOMEM;
		goto out;
	}

	cdev_init(&ram_devp->cdev, &ram_fops);
	ram_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&ram_devp->cdev, ram_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		/*err = err; */
		goto out;
	} else {
		cdev_add_done = 1;
	}

	/* send uevents to udev, so it'll create /dev nodes */
	/* /dev/ram0 */
	ram_device = device_create(ram_class, NULL, MKDEV(MAJOR(ram_major), 0),
					NULL, "ram%d", 0);

	return 0;  /* success */

out:
	ram_cleanup();
	return err;
}

static void __exit ram_exit(void)
{
	pr_debug("ram_exit()\n");

	ram_cleanup();
}

module_init(ram_init);
module_exit(ram_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
