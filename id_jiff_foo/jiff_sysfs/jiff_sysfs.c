
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

static ssize_t jiff_show(struct kobject *kobj, struct kobj_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%lu\n", jiffies);
}

static struct kobj_attribute jiff_attribute =
	__ATTR(jiffies, 0444, jiff_show, NULL);

static struct attribute *attrs[] = {
	&jiff_attribute.attr,
	NULL
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init jiff_sysfs_init(void)
{
	int ret;

	kobj = kobject_create_and_add("jiff_sysfs", kernel_kobj);
	if (!kobj)
		return -ENOMEM;

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret)
		kobject_put(kobj);

	return ret;
}

static void __exit jiff_sysfs_exit(void)
{
	kobject_put(kobj);
}

module_init(jiff_sysfs_init);
module_exit(jiff_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

