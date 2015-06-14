
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>

const char id[] = "aeda58c25c67";
#define ID_LEN (ARRAY_SIZE(id) - 1)

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%s\n", id);
}

/* return count if written id is correct, otherwise return -EINVAL */
static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
					const char *buf, size_t count)
{
	/* an id with or without a new-line is OK */

	if (count < ID_LEN || count > ID_LEN + 1)
		return -EINVAL;

	if (count == ID_LEN + 1 && buf[ID_LEN] != '\n')
		return -EINVAL;

	if (strncmp(id, buf, ID_LEN))
		return -EINVAL;

	return count;  /* correct */
}

static struct kobj_attribute id_attribute =
	__ATTR(id, 0666, id_show, id_store);

static ssize_t jiff_show(struct kobject *kobj, struct kobj_attribute *attr,
					char *buf)
{
	return sprintf(buf, "%lu\n", jiffies);
}

static struct kobj_attribute jiff_attribute =
	__ATTR(jiffies, 0444, jiff_show, NULL);

void *foo_data;
#define FOO_DATA_SIZE PAGE_SIZE

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
					char *buf)
{
	return snprintf(buf, FOO_DATA_SIZE, "%s", (char *) foo_data);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
					const char *buf, size_t count)
{
	return snprintf(foo_data, FOO_DATA_SIZE, "%s", buf);
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0644, foo_show, foo_store);

static struct attribute *attrs[] = {
	&id_attribute.attr,
	&jiff_attribute.attr,
	&foo_attribute.attr,
	NULL
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init ijf_sysfs_init(void)
{
	int ret = -ENOMEM;  /* default return value */

	foo_data = kmalloc(FOO_DATA_SIZE, GFP_KERNEL);
	if (!foo_data)
		goto err_malloc_foo;

	kobj = kobject_create_and_add("ijf_sysfs", kernel_kobj);
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

static void __exit ijf_sysfs_exit(void)
{
	kobject_put(kobj);
	kfree(foo_data);
}

module_init(ijf_sysfs_init);
module_exit(ijf_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
