
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "misc"

const char misc_id[] = "aeda58c25c67";

static ssize_t misc_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos, misc_id,
			ARRAY_SIZE(misc_id) - 1);
}

static ssize_t misc_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	char kbuf[ARRAY_SIZE(misc_id) - 1];

	if (count != ARRAY_SIZE(misc_id) - 1)
		return -EINVAL;

	if (!simple_write_to_buffer(kbuf, ARRAY_SIZE(kbuf), f_pos, buf, count))
		return -EINVAL;

	if (!strncmp(misc_id, kbuf, ARRAY_SIZE(misc_id) - 1))
		return -EINVAL;

	return count;  /* success */
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

