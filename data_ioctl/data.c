
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


int x;

static int data_open(struct inode *inode, struct file *filp)
{
	struct data_dev *data_devp;

	data_devp = container_of(inode->i_cdev, struct data_dev, cdev);

	filp->private_data = data_devp;

	return 0;
}

#define DATA_IOC_MAGIC 'm'

#define DATA_IOCRESET 	_IO(DATA_IOC_MAGIC,  1)
#define DATA_IOCWX	_IOW(DATA_IOC_MAGIC, 2, int)
#define DATA_IOCRX	_IOR(DATA_IOC_MAGIC, 3, int)

#define DATA_IOC_MAXNR 3

static long data_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	int err = 0;
	int retval = 0;

	/*struct data_dev *data_dev = filp->private_data; */

	if (_IOC_TYPE(cmd) != DATA_IOC_MAGIC) {
		pr_alert("invalid ioctl magic\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > DATA_IOC_MAXNR) {
		pr_alert("ioctl beyond maximum\n");
		return -ENOTTY;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg,
					_IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *) arg,
					_IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch (cmd) {
	case DATA_IOCRESET:
		/* takes no argument, sets values to default */
		x = 0;
		break;
	case DATA_IOCRX:
		/* read integer */
		retval = __put_user(x, (int __user *) arg);
		break;
	case DATA_IOCWX:
		/* write integer */
		retval = __get_user(x, (int __user *) arg);
		break;
	default:
		return -ENOTTY;  /* POSIX standard */
		/*return -EINVAL; */  /* common */
		/* Pg. 161 Linux Device Drivers (2005) */
	}

	return retval;
}

int data_release(struct inode *inode, struct file *filp)
{
	return 0;
}

const struct file_operations data_fops = {
	.owner = THIS_MODULE,
	.open = data_open,
	.unlocked_ioctl = data_ioctl,
	.release = data_release,
};

static int __init data_init(void)
{
	int err = 0;

	x = 0;

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
		goto err_malloc_data_devp;
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

module_init(data_init);
module_exit(data_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

