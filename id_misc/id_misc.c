
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "id"

const char id[] = "aeda58c25c67";
#define ID_LEN (ARRAY_SIZE(id) - 1)

static ssize_t id_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos, id,
			ID_LEN);
}

/* return count if written id is correct, otherwise return -EINVAL */
static ssize_t id_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	char kbuf[ID_LEN];

	if (count != ID_LEN)
		return -EINVAL;

	if (!simple_write_to_buffer(kbuf, ARRAY_SIZE(kbuf), f_pos, buf, count))
		return -EINVAL;

	if (strncmp(id, kbuf, ID_LEN))
		return -EINVAL;

	return count;  /* correct */
}

static const struct file_operations id_fops = {
	.owner = THIS_MODULE,
	.read  = id_read,
	.write = id_write,
};

static struct miscdevice id_dev = {
	.name  = DEVICE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops  = &id_fops,
};

static int __init id_init(void)
{
	return misc_register(&id_dev);
}

static void __exit id_exit(void)
{
	misc_deregister(&id_dev);
}

module_init(id_init);
module_exit(id_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

