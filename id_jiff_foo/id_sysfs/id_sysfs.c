
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/string.h>

#define MODULE_NAME "id_sysfs"

const char id[] = "aeda58c25c67";
#define ID_LEN (ARRAY_SIZE(id) - 1)

static ssize_t id_sysfs_show(struct kobject *kobj, struct attribute *attr,
					char *buf)
{
	return sprintf(buf, "%s\n", id);
}

/* return count if written id is correct, otherwise return -EINVAL */
static ssize_t id_sysfs_store(struct kobject *kobj, struct attribute *attr,
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

static const struct sysfs_ops id_sysfs_ops = {
	.show  = id_sysfs_show,
	.store = id_sysfs_store,
};

static struct kobj_type id_sysfs_ktype = {
	.sysfs_ops = &id_sysfs_ops,
};

struct attribute id_sysfs_attr = {
	.name = "id",
	.mode = 0666,
};

static struct attribute *attrs[] = {
	&id_sysfs_attr,
	NULL
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

struct kobject *kobj;

static int __init id_sysfs_init(void)
{
	int ret;

	kobj = kobject_create_and_add(MODULE_NAME, kernel_kobj);
	if (!kobj)
		return -ENOMEM;

	kobj->ktype = &id_sysfs_ktype;

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

