
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
	size_t left;

	if (*f_pos >= strlen_misc_id)
		return 0;  /* EOF */

	left = strlen_misc_id - *f_pos;

	if (count > left)
		count = left;

	if (copy_to_user(buf, misc_id + *f_pos, count) != 0)
		return -EIO;

	*f_pos += count;

	return count;
}

static ssize_t misc_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	char kbuf[strlen_misc_id];

	if (count != strlen_misc_id)
		return -EINVAL;

	if (copy_from_user(kbuf, buf, strlen_misc_id) != 0)
		return -EIO;

	if (0 != strncmp(misc_id, kbuf, strlen_misc_id))
		return -EINVAL;

	return count;
}

const struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.read = misc_read,
	.write = misc_write,
};

static struct miscdevice misc_dev;

static int __init misc_init(void)
{
	misc_dev.name = DEVICE_NAME;
	misc_dev.minor = MISC_DYNAMIC_MINOR;
	misc_dev.fops = &misc_fops;

	return misc_register(&misc_dev);
}

static void __exit misc_exit(void)
{
	misc_deregister(&misc_dev);

	return;
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

