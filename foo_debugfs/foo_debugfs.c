
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#define MODULE_NAME "foo_debugfs"

static struct dentry *root;

void *foo_data;
#define FOO_DATA_SIZE PAGE_SIZE
DEFINE_MUTEX(foo_lock);

static ssize_t foo_read(struct file *filp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	int ret;

	if (mutex_lock_interruptible(&foo_lock))
		return -EINTR;
	ret = simple_read_from_buffer(buf, count, f_pos,
					foo_data, FOO_DATA_SIZE);
	mutex_unlock(&foo_lock);

	return ret;
}

static ssize_t foo_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *f_pos)
{
	int ret;

	if (mutex_lock_interruptible(&foo_lock))
		return -EINTR;
	ret = simple_write_to_buffer(foo_data, FOO_DATA_SIZE,
					f_pos, buf, count);
	mutex_unlock(&foo_lock);
	if (!ret)
		return -EIO;

	return count;
}

static const struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.read  = foo_read,
	.write = foo_write,
};

static int __init foo_debugfs_init(void)
{
	struct dentry *foo;

	foo_data = kmalloc(FOO_DATA_SIZE, GFP_KERNEL);
	if (!foo_data)
		goto err_malloc_foo;

	root = debugfs_create_dir(MODULE_NAME, NULL);
	if (!root)
		goto err_debugfs_root;

	foo = debugfs_create_file("foo", 0644, root, NULL, &foo_fops);
	if (!foo)
		goto err_debugfs_files;

	return 0;

err_debugfs_files:
	debugfs_remove_recursive(root);
err_debugfs_root:
	kfree(foo_data);
err_malloc_foo:
	return -ENOMEM;
}

static void __exit foo_debugfs_exit(void)
{
	debugfs_remove_recursive(root);
	kfree(foo_data);
}

module_init(foo_debugfs_init);
module_exit(foo_debugfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

