
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>

void *foo_data;
#define FOO_DATA_SIZE PAGE_SIZE
DEFINE_MUTEX(foo_lock);

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
					char *buf)
{
	int ret;

	if (mutex_lock_interruptible(&foo_lock))
		return -EINTR;
	ret = snprintf(buf, FOO_DATA_SIZE, "%s", (char *) foo_data);
	mutex_unlock(&foo_lock);

	return ret;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
					const char *buf, size_t count)
{
	int ret;

	if (mutex_lock_interruptible(&foo_lock))
		return -EINTR;
	ret = snprintf(foo_data, FOO_DATA_SIZE, "%s", buf);
	mutex_unlock(&foo_lock);

	return ret;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0644, foo_show, foo_store);

static struct attribute *attrs[] = {
	&foo_attribute.attr,
	NULL
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init foo_sysfs_init(void)
{
	int ret = -ENOMEM;  /* default return value */

	foo_data = kmalloc(FOO_DATA_SIZE, GFP_KERNEL);
	if (!foo_data)
		goto err_malloc_foo;

	kobj = kobject_create_and_add("foo_sysfs", kernel_kobj);
	if (!kobj)
		goto err_kobj_create;

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret)
		goto err_sysfs_group;

	return 0;

err_sysfs_group:
	kobject_put(kobj);
err_kobj_create:
	kfree(foo_data);
err_malloc_foo:
	return ret;
}

static void __exit foo_sysfs_exit(void)
{
	kobject_put(kobj);
	kfree(foo_data);
}

module_init(foo_sysfs_init);
module_exit(foo_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

