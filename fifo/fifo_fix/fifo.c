
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "fifo"
#define MAX_DATA 3

static dev_t fifo_major;
struct class *fifo_class;
struct device *fifo_device;

struct fifo_dev {
	struct cdev cdev;
	char fifo[MAX_DATA];
	char *read_ptr;
	char *write_ptr;
	char *fifo_start;
	char *fifo_end;
	int empty;
	struct mutex ferw_mtx;  /* ferw = fifo, empty, read, write */
} *fifo_devp;

int fifo_open(struct inode *inode, struct file *filp)
{
	struct fifo_dev *fifo_devp;

	fifo_devp = container_of(inode->i_cdev, struct fifo_dev, cdev);

	filp->private_data = fifo_devp;

	return 0;
}

#define DEFINE_PWAIT(uid)						\
									\
static DEFINE_MUTEX(in_mtx_##uid);					\
static DEFINE_MUTEX(out_mtx_##uid);					\
									\
static void pwait_##uid(void)						\
{									\
	static int in;							\
	static int out;							\
									\
	mutex_lock(&in_mtx_##uid);					\
	if (++in >= 2) {						\
		mutex_lock(&out_mtx_##uid);				\
		out += 2;						\
		mutex_unlock(&out_mtx_##uid);				\
		in -= 2;						\
	}								\
	mutex_unlock(&in_mtx_##uid);					\
									\
	do {								\
		mutex_lock(&out_mtx_##uid);				\
		if (out) {						\
			out -= 1;					\
			mutex_unlock(&out_mtx_##uid);			\
			break;						\
		}							\
		mutex_unlock(&out_mtx_##uid);				\
	} while (1);							\
}

DEFINE_PWAIT(fifo_read);

static ssize_t fifo_read(struct file *filp, char __user *buf,
						size_t count, loff_t *f_pos)
{
	struct fifo_dev *dev = filp->private_data;
	size_t left;

	left = count;

	while (left) {

		pwait_fifo_read();

		mutex_lock(&dev->ferw_mtx);

		if (dev->empty) {
			mutex_unlock(&dev->ferw_mtx);
			break;
		}

		if (copy_to_user(buf, (void *) dev->read_ptr, 1) != 0) {
			mutex_unlock(&dev->ferw_mtx);
			return -EIO;
		}
		left--;

		if (dev->read_ptr == dev->fifo_end)
			dev->read_ptr = dev->fifo_start;
		else
			(dev->read_ptr)++;

		if (dev->read_ptr == dev->write_ptr)
			dev->empty = 1;

		mutex_unlock(&dev->ferw_mtx);
	}

	return (count - left);
}

DEFINE_PWAIT(fifo_write);

static ssize_t fifo_write(struct file *filp, const char __user *buf,
						size_t count, loff_t *f_pos)
{
	struct fifo_dev *dev = filp->private_data;
	size_t left;

	left = count;

	while (left) {

		pwait_fifo_write();

		mutex_lock(&dev->ferw_mtx);

		if (!(dev->empty) && (dev->read_ptr == dev->write_ptr)) {
			mutex_unlock(&dev->ferw_mtx);
			break;
		}

		if (copy_from_user((void *) dev->write_ptr, buf, 1) != 0) {
			mutex_unlock(&dev->ferw_mtx);
			return -EIO;
		}
		left--;

		if (dev->empty)
			dev->empty = 0;

		if (dev->write_ptr == dev->fifo_end)
			dev->write_ptr = dev->fifo_start;
		else
			(dev->write_ptr)++;
		mutex_unlock(&dev->ferw_mtx);
	}

	return (count - left);
}

static int fifo_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations fifo_fops = {
	.owner = THIS_MODULE,
	.open = fifo_open,
	.read = fifo_read,
	.write = fifo_write,
	.release = fifo_release,
};


static ssize_t read_offset_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct fifo_dev *fifo_devp = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n",
			(unsigned int) (fifo_devp->read_ptr -
				fifo_devp->fifo_start));
}

static ssize_t read_offset_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	return 0;  /* cannot store anything */
}

static DEVICE_ATTR(read_offset, 0666, read_offset_show, read_offset_store);

static ssize_t write_offset_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct fifo_dev *fifo_devp = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", (unsigned int) (fifo_devp->write_ptr -
					fifo_devp->fifo_start));
}

static ssize_t write_offset_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf,
					size_t count)
{
	return 0;  /* cannot store anything */
}

static DEVICE_ATTR(write_offset, 0666, write_offset_show, write_offset_store);


static int __init fifo_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&fifo_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_warn("Unable to register device\n");
		goto err_chrdev_region;
	}

	fifo_class = class_create(THIS_MODULE, DEVICE_NAME);

	fifo_devp = kmalloc(sizeof(struct fifo_dev), GFP_KERNEL);
	if (!fifo_devp) {
		pr_warn("Unable to kmalloc fifo_devp\n");
		err = -ENOMEM;
		goto err_malloc_devp;
	}

	cdev_init(&fifo_devp->cdev, &fifo_fops);
	fifo_devp->cdev.owner = THIS_MODULE;

	fifo_devp->fifo_start = &fifo_devp->fifo[0];
	fifo_devp->fifo_end = &fifo_devp->fifo[MAX_DATA-1];
	fifo_devp->read_ptr = &fifo_devp->fifo[0];
	fifo_devp->write_ptr = &fifo_devp->fifo[0];
	fifo_devp->empty = 1;

	mutex_init(&fifo_devp->ferw_mtx);

	err = cdev_add(&fifo_devp->cdev, fifo_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		goto err_cdev_add;
	}

	fifo_device = device_create(fifo_class, NULL,
				MKDEV(MAJOR(fifo_major), 0), NULL, "fifo%d", 0);
	if (IS_ERR(fifo_device)) {
		pr_warn("device_create failed\n");
		err = PTR_ERR(fifo_device);
		goto err_device_create;
	}

	err = dev_set_drvdata(fifo_device, fifo_devp);
	if (err)
		goto err_set_drvdata;

	err = device_create_file(fifo_device, &dev_attr_read_offset);
	if (err)
		goto err_file_read_offset;

	err = device_create_file(fifo_device, &dev_attr_write_offset);
	if (err)
		goto err_file_write_offset;

	return 0;  /* success */

err_file_write_offset:
	/* device_remove_file(fifo_device, &dev_attr_write_offset); */
err_file_read_offset:
	device_remove_file(fifo_device, &dev_attr_read_offset);
err_set_drvdata:
	device_destroy(fifo_class, fifo_major);
err_device_create:
	cdev_del(&fifo_devp->cdev);
err_cdev_add:
	kfree(fifo_devp);
err_malloc_devp:
	class_destroy(fifo_class);
	unregister_chrdev_region(fifo_major, 1);
err_chrdev_region:

	return err;
}

static void __exit fifo_exit(void)
{
	device_remove_file(fifo_device, &dev_attr_write_offset);

	device_remove_file(fifo_device, &dev_attr_read_offset);

	device_destroy(fifo_class, fifo_major);

	cdev_del(&fifo_devp->cdev);

	kfree(fifo_devp);

	class_destroy(fifo_class);

	unregister_chrdev_region(fifo_major, 1);
}

module_init(fifo_init);
module_exit(fifo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

