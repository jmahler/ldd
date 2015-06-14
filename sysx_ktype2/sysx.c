
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
	if (0 == strcmp(attr->name, "x"))
		return sprintf(buf, "%d\n", x);
	else
		return sprintf(buf, "%d\n", y);
}

static ssize_t sysx_xy_store(struct kobject *kobj, struct attribute *attr,
						const char *buf, size_t count)
{
	int ret;

	if (0 == strcmp(attr->name, "x"))
		ret = sscanf(buf, "%du", &x);
	else
		ret = sscanf(buf, "%du", &y);

	if (ret != 1)
		return -EINVAL;

	return count;
}

void sysx_xy_release(struct kobject *kobj)
{
}

static const struct sysfs_ops sysx_sysfs_ops = {
	.show = sysx_xy_show,
	.store = sysx_xy_store,
};

static struct kobj_type sysx_ktype = {
	.release = sysx_xy_release,
	.sysfs_ops = &sysx_sysfs_ops,
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
	NULL,  /* terminate list */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init sysx_init(void)
{
	int ret;

	kobj = kzalloc(sizeof(*kobj), GFP_KERNEL);
	if (!kobj)
		return -ENOMEM;

	kobj->ktype = &sysx_ktype;

	ret = kobject_init_and_add(kobj, &sysx_ktype, kernel_kobj,
						"%s", "sysx");
	if (ret)
		kobject_put(kobj);

	ret = sysfs_create_group(kobj, &attr_group);
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
