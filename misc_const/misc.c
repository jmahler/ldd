
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "misc"

const char misc_id[] = "aeda58c25c67";
#define strlen_misc_id (sizeof(misc_id) - 1)  /* w/o '\0' */

static ssize_t misc_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	int mcount = min(strlen_misc_id, count);

	return copy_to_user(buf, misc_id, mcount) ? -EIO : mcount;
}

static ssize_t misc_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	char kbuf[strlen_misc_id];

	if (count != strlen_misc_id)
		return -EINVAL;

	if (copy_from_user(kbuf, buf, count) != 0)
		return -EIO;

	if (strncmp(misc_id, buf, count))
		return -EINVAL;

	return count;
}

static const struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.read = misc_read,
	.write = misc_write,
};

static struct miscdevice misc_dev = {
	.name = DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &misc_fops,
};

static int __init misc_init(void)
{
	return misc_register(&misc_dev);
}

static void __exit misc_exit(void)
{
	misc_deregister(&misc_dev);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

