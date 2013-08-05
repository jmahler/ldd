
#define DEVICE_NAME "sysx"

#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>

static int x;
static int y;

static ssize_t sysx_xy_show(struct kobject *kobj, struct attribute *attr,
						char *buf)
{
	if (0 == strcmp(attr->name, "x")) {
		return sprintf(buf, "%d\n", x);
	} else {
		return sprintf(buf, "%d\n", y);
	}
}

static ssize_t sysx_xy_store(struct kobject *kobj, struct attribute *attr,
						const char *buf, size_t count)
{
	if (0 == strcmp(attr->name, "x")) {
		sscanf(buf, "%du", &x);
	} else {
		sscanf(buf, "%du", &y);
	}

	return count;
}

static struct sysfs_ops sysx_sysfs_ops = {
	.show = sysx_xy_show,
	.store = sysx_xy_store,
};

struct attribute x_attr = {
	.name = "x",
	.mode = 0666,
};

struct attribute y_attr = {
	.name = "y",
	.mode = 0666,
};

static struct attribute *attrs[] = {
	&x_attr,
	&y_attr,
	NULL,  // terminate list
};

static struct kobj_type sysx_ktype = {
	.sysfs_ops = &sysx_sysfs_ops,
	.default_attrs = attrs,
};

struct kobject *kobj;

static int __init sysx_init(void)
{
	int ret;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (!kobj)
		return -ENOMEM;

	kobj->ktype = &sysx_ktype;

	ret = kobject_init_and_add(kobj, &sysx_ktype, NULL, "%s", "sysx");
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
