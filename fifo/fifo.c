
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "fifo"
#define BUF_SIZE 8192 

// per device structure
struct fifo_dev {
	struct cdev cdev;
	char buf[BUF_SIZE];
	int buf_rd_pos;  // read position in buffer
	int buf_wr_pos;  // write position in buffer
	bool buf_empty;  // true if buffer is empty
} *fifo_devp;

static dev_t fifo_dev_number;
struct class *fifo_class;

// {{{ fifo_open
int
fifo_open(struct inode *inode, struct file *file)
{
	struct fifo_dev *fifo_devp;

	fifo_devp = container_of(inode->i_cdev, struct fifo_dev, cdev);

	file->private_data = fifo_devp;

	return 0;  // OK
}
// }}}

// {{{ fifo_release
int
fifo_release(struct inode *inode, struct file *file)
{
	//struct fifo_dev *fifo_devp = file->private_data;

	return 0;  // OK
}
// }}}

// {{{ fifo_read
ssize_t
fifo_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct fifo_dev *fifo_devp = file->private_data;

	int n;
	int *buf_rd_pos = &fifo_devp->buf_rd_pos;
	int buf_wr_pos = fifo_devp->buf_wr_pos;

	n = 0;
	while (1) {
		// if reached number of requested value
		if (n >= count)
			break;

		if (fifo_devp->buf_empty)
			break;

		// read the value
		*buf = fifo_devp->buf[*buf_rd_pos];
		buf++;
		n++;

		// next position, check for overflow
		(*buf_rd_pos)++;
		if (*buf_rd_pos >= BUF_SIZE) {
			*buf_rd_pos = 0;
		}

		// if empty
		if (*buf_rd_pos == buf_wr_pos) {
			fifo_devp->buf_empty = true;
			break;
		}
	}

	return n;
}
// }}}

// {{{ fifo_write
ssize_t
fifo_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	struct fifo_dev *fifo_devp = file->private_data;
	int buf_rd_pos = fifo_devp->buf_rd_pos;
	int *buf_wr_pos = &fifo_devp->buf_wr_pos;
	bool *buf_empty = &fifo_devp->buf_empty;
	int n;

	n = 0;
	while (1) {

		// if reached number of requested value
		if (n >= count)
			break;

		// if full
		if (buf_rd_pos == *buf_wr_pos && ! *buf_empty) {
			break;
		}

		// write the value
		fifo_devp->buf[*buf_wr_pos] = *buf;
		buf++;
		n++;
		if (*buf_empty)
			*buf_empty = false;

		// next position, check for overflow
		(*buf_wr_pos)++;
		if (*buf_wr_pos >= BUF_SIZE) {
			*buf_wr_pos = 0;
		}
	}

	return n;
}
// }}}

static struct file_operations fifo_fops = {
	.owner    = THIS_MODULE,
	.open     = fifo_open,
	.release  = fifo_release,
	.read     = fifo_read,
	.write    = fifo_write,
//	.ioctl    = fifo_ioctl,
};

// {{{ fifo_init()
static int __init fifo_init(void)
{
	int ret;

	// allocate a device major number
	if (alloc_chrdev_region(&fifo_dev_number, 0, 1, DEVICE_NAME) < 0) {
		printk(KERN_ERR "Unable to allocate device major number\n");

		return -1;  // ERROR
	}

	// create a sysfs entry /sys/class/fifo
	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(fifo_class)) {
		printk(KERN_ERR "Unable to create fifo class memory\n");

		unregister_chrdev_region(fifo_dev_number, 0);

		return PTR_ERR(fifo_class);
	}

	// allocate memory
	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		printk(KERN_ERR "Unable to kmalloc memory\n");

		unregister_chrdev_region(fifo_dev_number, 0);
		class_destroy(fifo_class);

		return -ENOMEM;
	}

	// connect the file operations with the cdev
	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;

	// init our buffer
	fifo_devp->buf_rd_pos = 0;
	fifo_devp->buf_wr_pos = 0;
	fifo_devp->buf_empty = true;

	// add our cdev to the kernel
	ret = cdev_add(&fifo_devp->cdev, fifo_dev_number, 1);
	if (ret) {
		printk(KERN_ERR "Unable to add cdev\n");

		unregister_chrdev_region(fifo_dev_number, 0);
		class_destroy(fifo_class);
		kfree(fifo_devp);

		return ret;
	}

	// notify udev, so it will create the devices
	device_create(fifo_class, NULL, fifo_dev_number, NULL, "fifo0");

	return 0;  // OK
}
// }}}

// {{{ fifo_exit()
static void __exit fifo_exit(void)
{
	// release the device major number
	unregister_chrdev_region(fifo_dev_number, 0);

	// tell udev to destroy the device
	//device_destroy(fifo_class, MKDEV(MAJOR(fifo_dev_number), 0));
	device_destroy(fifo_class, fifo_dev_number);

	// remove our cdev from the kernel
	cdev_del(&fifo_devp->cdev);

	// free memory
	kfree(fifo_devp);

	// remove the sysfs entry /sys/class/fifo
	class_destroy(fifo_class);
}
// }}}

module_init(fifo_init);
module_exit(fifo_exit);

