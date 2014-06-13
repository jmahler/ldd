
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

#define DEVICE_NAME "hello"

const char hello_id[] = "aeda58c25c67";

struct hello_dev {
	struct dentry *root;
	struct dentry *id;
} *hello_devp;


static ssize_t hello_id_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos, hello_id,
			ARRAY_SIZE(hello_id) - 1);
}

static ssize_t hello_id_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	char kbuf[ARRAY_SIZE(hello_id) - 1];

	if (count != ARRAY_SIZE(hello_id) - 1)
		return -EINVAL;

	if (!simple_write_to_buffer(kbuf, ARRAY_SIZE(kbuf), f_pos, buf, count))
		return -EINVAL;

	if (!strncmp(hello_id, kbuf, ARRAY_SIZE(hello_id) - 1))
		return -EINVAL;

	return count;  /* success */
}

static const struct file_operations hello_id_fops = {
	.owner = THIS_MODULE,
	.read = hello_id_read,
	.write = hello_id_write,
};

static int __init hello_init(void)
{
	hello_devp = kmalloc(sizeof(struct hello_dev), GFP_KERNEL);
	if (!hello_devp)
		goto err_malloc_devp;

	hello_devp->root = debugfs_create_dir(DEVICE_NAME, NULL);
	if (!hello_devp->root)
		goto err_debugfs_root;

	hello_devp->id = debugfs_create_file("id", 0666,
					hello_devp->root, hello_devp,
					&hello_id_fops);
	if (!hello_devp->id)
		goto err_debugfs_files;

	return 0;

err_debugfs_files:
	debugfs_remove_recursive(hello_devp->root);
err_debugfs_root:
	kfree(hello_devp);
err_malloc_devp:
	return -ENOMEM;
}

static void __exit hello_exit(void)
{
	debugfs_remove_recursive(hello_devp->root);
	kfree(hello_devp);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

