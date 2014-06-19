
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#define DEVICE_NAME "dbgid"

const char id[] = "aeda58c25c67";
#define ID_SIZE (ARRAY_SIZE(id) - 1)

static struct dentry *root;

static ssize_t id_read(struct file *filp, char __user *buf,
					size_t count, loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos, id, ID_SIZE);
}

/* return -EINVAL if incorrect id is written, return count if correct */
static ssize_t id_write(struct file *filp, const char __user *buf,
					size_t count, loff_t *f_pos)
{
	char kbuf[ID_SIZE];

	if (count != ID_SIZE)
		return -EINVAL;

	if (!simple_write_to_buffer(kbuf, ARRAY_SIZE(kbuf), f_pos, buf, count))
		return -EINVAL;

	if (strncmp(id, kbuf, ID_SIZE))
		return -EINVAL;

	return count;  /* correct */
}

static const struct file_operations id_fops = {
	.owner = THIS_MODULE,
	.read  = id_read,
	.write = id_write,
};

static int __init hello_init(void)
{
	struct dentry *id;

	root = debugfs_create_dir(DEVICE_NAME, NULL);
	if (!root)
		goto err_debugfs_root;

	id = debugfs_create_file("id", 0666, root, NULL, &id_fops);
	if (!id)
		goto err_debugfs_files;

	return 0;

err_debugfs_files:
	debugfs_remove_recursive(root);
err_debugfs_root:
	return -ENOMEM;
}

static void __exit hello_exit(void)
{
	debugfs_remove_recursive(root);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

