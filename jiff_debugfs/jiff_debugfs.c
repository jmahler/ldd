
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/module.h>

#define MODULE_NAME "jiff_debugfs"

static struct dentry *root;

static int __init jiff_debugfs_init(void)
{
	struct dentry *jiff;

	root = debugfs_create_dir(MODULE_NAME, NULL);
	if (!root)
		goto err_debugfs_root;

	jiff = debugfs_create_u64("jiffies", 0444, root, (u64 *) &jiffies);
	if (!jiff)
		goto err_debugfs_files;

	return 0;

err_debugfs_files:
	debugfs_remove_recursive(root);
err_debugfs_root:
	return -ENOMEM;
}

static void __exit jiff_debugfs_exit(void)
{
	debugfs_remove_recursive(root);
}

module_init(jiff_debugfs_init);
module_exit(jiff_debugfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");

