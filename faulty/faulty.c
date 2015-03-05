/*
 * NAME
 *
 * faulty - Driver which creates a fault during a read/write.
 *
 * DESCRIPTION
 *
 * This module will generate a fault during a read or write.
 *
 *   sudo dd if=/dev/faulty of=/dev/null count=20
 *
 *   (output in logs)
 *   Mar 04 20:42:07 frost sudo[2535]: pam_unix(sudo:session): session opened for user root by jeri(uid=0)
 *   Mar 04 20:42:07 frost kernel: BUG: unable to handle kernel paging request at ffffffffffffffff
 *   Mar 04 20:42:07 frost kernel: IP: [<ffffffffffffffff>] 0xffffffffffffffff
 *   Mar 04 20:42:07 frost kernel: PGD 1810067 PUD 1812067 PMD 0 
 *   Mar 04 20:42:07 frost kernel: Oops: 0010 [#1] SMP 
 *   ...
 *
 * A similar fault will occur during a write.
 *
 *   sudo dd if=/dev/null of=/dev/faulty count=20
 *
 * After the fault occurs it may be necessary to reboot the machine to get
 * it to occur again.
 *
 * This driver is based on the example given on Page 113 of
 * Linux Device Drivers [1].
 *
 *   [1]: http://lwn.net/Kernel/LDD3/
 */

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "faulty"

static dev_t faulty_major;
struct class *faulty_class;
struct device *faulty_device;

struct faulty_dev {
	struct cdev cdev;
} *faulty_devp;


static int faulty_open(struct inode *inode, struct file *filp)
{
	struct faulty_dev *faulty_devp;

	faulty_devp = container_of(inode->i_cdev, struct faulty_dev, cdev);

	filp->private_data = faulty_devp;

	return 0;
}

static ssize_t faulty_read(struct file *filp, char __user *buf,
					size_t count, loff_t *f_pos)
{
	int ret;
	char stack_buf[4];

	/* Create a buffer overflow fault */
	memset(stack_buf, 0xff, 20);

	if (count > 4)
		count = 4;

	ret = copy_to_user(buf, stack_buf, count);
	if (!ret)
		return count;

	return ret;
}

static ssize_t faulty_write(struct file *filp, const char __user *buf,
					size_t count, loff_t *f_pos)
{
	/* Create a fault by trying to de-reference a NULL pointer */
	*(int *)0 = 0;
	return 0;
}

static int faulty_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations faulty_fops = {
	.owner = THIS_MODULE,
	.open = faulty_open,
	.read = faulty_read,
	.write = faulty_write,
	.release = faulty_release,
};

static int __init faulty_init(void)
{
	int err = 0;

	err = alloc_chrdev_region(&faulty_major, 0, 1, DEVICE_NAME);
	if (err < 0) {
		pr_warn("Unable to register device\n");
		goto err_chrdev_region;
	}

	faulty_class = class_create(THIS_MODULE, DEVICE_NAME);

	faulty_devp = kmalloc(sizeof(struct faulty_dev), GFP_KERNEL);
	if (!faulty_devp) {
		pr_warn("Unable to kmalloc faulty_devp\n");
		err = -ENOMEM;
		goto err_malloc_faulty_devp;
	}

	cdev_init(&faulty_devp->cdev, &faulty_fops);
	faulty_devp->cdev.owner = THIS_MODULE;
	err = cdev_add(&faulty_devp->cdev, faulty_major, 1);
	if (err) {
		pr_warn("cdev_add failed\n");
		goto err_cdev_add;
	}

	faulty_device = device_create(faulty_class, NULL,
				MKDEV(MAJOR(faulty_major), 0), NULL,
				"faulty");
	if (IS_ERR(faulty_device)) {
		pr_warn("device_create failed\n");
		err = PTR_ERR(faulty_device);
		goto err_device_create;
	}

	return 0;

err_device_create:
	cdev_del(&faulty_devp->cdev);
err_cdev_add:
	kfree(faulty_devp);
err_malloc_faulty_devp:
	class_destroy(faulty_class);
	unregister_chrdev_region(faulty_major, 1);
err_chrdev_region:

	return err;
}

static void __exit faulty_exit(void)
{
	device_destroy(faulty_class, faulty_major);

	cdev_del(&faulty_devp->cdev);

	kfree(faulty_devp);

	class_destroy(faulty_class);

	unregister_chrdev_region(faulty_major, 1);
}

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");

module_init(faulty_init);
module_exit(faulty_exit);
