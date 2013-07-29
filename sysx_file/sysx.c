
#define DEVICE_NAME "sysx"

#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

static int x;

/* operations to read (x_show) and write (x_store) the file for 'x' in /sys */
static ssize_t x_show(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
	return sprintf(buf, "%d\n", x);
}

static ssize_t x_store(struct kobject *kobj, struct kobj_attribute *attr,
						const char *buf, size_t count)
{
	sscanf(buf, "%du", &x);
	return count;
}

/* "attributes" refer to files in kobject lingo */
static struct kobj_attribute x_attribute =
	__ATTR(x, 0666, x_show, x_store);

struct kobject *kobj;

static int __init sysx_init(void)
{
	int ret;

	/*
	 * /sys/kernel/sysx
	 *        |     |
	 *        |    "sysx"
	 *        |
	 *       kernel_'kobj'
	 */
	kobj = kobject_create_and_add("sysx", kernel_kobj);
	if (!kobj)
		return -ENOMEM;

	/* /sys/kernel/sysx/x */
	ret = sysfs_create_file(kobj, &x_attribute.attr);

	if (ret)
		kobject_put(kobj);

	return ret;
}

static void __exit sysx_exit(void)
{
	kobject_put(kobj);
}

module_init(sysx_init);
module_exit(sysx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
