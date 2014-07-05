
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>

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

static struct attribute *attrs[] = {
	&id_attribute.attr,
	NULL
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init id_sysfs_init(void)
{
	int ret;

	kobj = kobject_create_and_add("id_sysfs", kernel_kobj);
	if (!kobj)
		return -ENOMEM;

	ret = sysfs_create_group(kobj, &attr_group);
	if (ret)
		kobject_put(kobj);

	return ret;
}

static void __exit id_sysfs_exit(void)
{
	kobject_put(kobj);
}

module_init(id_sysfs_init);
module_exit(id_sysfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

