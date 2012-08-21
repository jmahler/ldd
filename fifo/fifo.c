
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "fifo"
#define FIFO_SIZE 8192 

// per device structure
struct fifo_dev {
	struct cdev cdev;
	char *buf;
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
fifo_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	// ubuf - user space buffer

	struct fifo_dev *fifo_devp = file->private_data;

	int *buf_rd_pos = &fifo_devp->buf_rd_pos;
	int buf_wr_pos = fifo_devp->buf_wr_pos;
	int num_left;
	int max_count;  // result count, might be less than 'count'
	int n;
	char *kbuf = fifo_devp->buf;

	// 0 1 2 3 4 5 6 7
	// - | - - - - | -  = 5
	//  rd        wr
	//
	// - | - - - - | -  = 3
	//  wr        rd
	//
	// - | - - - - - -  = 0 if empty, 8 if not empty
	//  wr
	//  rd
	//
	// Calculate the number left to read which is
	// the difference between the write position and the
	// read position.  And it may have overflowed relative
	// to the FIFO_SIZE.
	if (buf_wr_pos > *buf_rd_pos) {
		num_left = buf_wr_pos - *buf_rd_pos;
	} else if (buf_wr_pos == *buf_rd_pos) {
		if (fifo_devp->buf_empty)
			num_left = 0;
		else
			num_left = FIFO_SIZE;
	} else {
		num_left = buf_wr_pos + (FIFO_SIZE - *buf_rd_pos); 
	}
	max_count = (count > num_left) ? num_left : count;

//	printk("fifo_read PRE:\n");
//	printk("  num_left = %u\n", num_left);
//	printk("  max_count = %u\n", max_count);
//	printk("  buf_rd_pos = %u\n", *buf_rd_pos);
//	printk("  buf_wr_pos = %u\n", buf_wr_pos);
//	printk("  empty = %u\n", fifo_devp->buf_empty);

	// move the pointer start of the kernel buffer for copy_to_user()
	kbuf += *buf_rd_pos;

	n = copy_to_user((void __user *) buf, kbuf, max_count);

	// on success
	if (0 == n) {
		// update read position, account for overflow
		*buf_rd_pos += max_count;
		if (*buf_rd_pos >= FIFO_SIZE)
			*buf_rd_pos = *buf_rd_pos - FIFO_SIZE;

		if (max_count == num_left)
			fifo_devp->buf_empty = true;

//	printk("fifo_read POST:\n");
//	printk("  buf_rd_pos = %u\n", *buf_rd_pos);
//	printk("  buf_wr_pos = %u\n", buf_wr_pos);
//	printk("  empty = %u\n", fifo_devp->buf_empty);

		return max_count;  // OK
	}

	return 0;  // ERROR - nothing transferred
}
// }}}

// {{{ fifo_write
ssize_t
fifo_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct fifo_dev *fifo_devp = file->private_data;

	int buf_rd_pos = fifo_devp->buf_rd_pos;
	int *buf_wr_pos = &fifo_devp->buf_wr_pos;
	int num_left;
	int max_count;  // result count, might be less than 'count'
	int n;
	char *kbuf = fifo_devp->buf;

	// 0 1 2 3 4 5 6 7
	// - | - - - - | -  = 3
	//  rd        wr
	//
	// - | - - - - | -  = 5
	//  wr        rd
	//
	// - | - - - - - -  = 8 if empty, 0 if not empty
	//  wr
	//  rd
	//
	// Calculate the number left to write.
	// Difference between the write position and the read position.
	// And the numbers may have overflowed.
	if (buf_rd_pos > *buf_wr_pos) {
		num_left = buf_rd_pos - *buf_wr_pos;
	} else if (buf_rd_pos == *buf_wr_pos) {
		if (fifo_devp->buf_empty)
			num_left = FIFO_SIZE;
		else
			num_left = 0;
	} else {
		num_left = buf_rd_pos + (FIFO_SIZE - *buf_wr_pos); 
	}

	max_count = (count > num_left) ? num_left : count;

//	printk("fifo_write PRE:\n");
//	printk("  num_left = %u\n", num_left);
//	printk("  max_count = %u\n", max_count);
//	printk("  buf_rd_pos = %u\n", buf_rd_pos);
//	printk("  buf_wr_pos = %u\n", *buf_wr_pos);
//	printk("  empty = %u\n", fifo_devp->buf_empty);

	kbuf += *buf_wr_pos;

	n = copy_from_user(kbuf, (char __user *) buf, max_count);

	if (0 == n) {
		*buf_wr_pos += max_count;
		if (*buf_wr_pos >= FIFO_SIZE)
			*buf_wr_pos = *buf_wr_pos - FIFO_SIZE;

		if (max_count > 0)
			fifo_devp->buf_empty = false;

//	printk("fifo_write POST:\n");
//	printk("  buf_rd_pos = %u\n", buf_rd_pos);
//	printk("  buf_wr_pos = %u\n", *buf_wr_pos);
//	printk("  empty = %u\n", fifo_devp->buf_empty);

		return max_count;  // OK
	}

	return 0;  // ERROR - nothing transferred
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

	// allocate the buffer
	fifo_devp->buf = kmalloc(FIFO_SIZE, GFP_KERNEL);
	if (!fifo_devp->buf) {
		printk(KERN_ERR "Unable to kmalloc memory for buffer\n");

		unregister_chrdev_region(fifo_dev_number, 0);
		class_destroy(fifo_class);
		kfree(fifo_devp);

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
	kfree(fifo_devp->buf);
	kfree(fifo_devp);

	// remove the sysfs entry /sys/class/fifo
	class_destroy(fifo_class);
}
// }}}

module_init(fifo_init);
module_exit(fifo_exit);

