/*
 * NAME
 * ----
 *
 * sequence - Read an endless number sequence from a debugfs file.
 *
 * DESCRIPTION
 * -----------
 *
 * Creates a debugfs file from which an endless incrementing sequence
 * of numbers can be read.
 *
 *   $ cat /sys/kernel/debug/sequence
 *   0
 *   1
 *   2
 *   3
 *   ...
 *
 * It serves as an example of how to use the seq_file interface.
 * It is based on the article "Driver porting: The seq_file interface"
 * by Jonathan Corbet [1].
 *
 * AUTHOR
 * ------
 *
 *  Jeremiah Mahler <jmmahler@gmail.com>
 *
 * REFERENCES
 * ----------
 *
 *  [1] Driver porting: The seq_file interface. Corbet.
 *  	https://lwn.net/Articles/22355/
 *
 *  [2] http://rostedt.homelinux.com/private/tasklist.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

static struct dentry *debugfs_file;

static void *sequence_start(struct seq_file *s, loff_t *pos)
{
	return (void *) pos;
}

static void *sequence_next(struct seq_file *s, void *v, loff_t *pos)
{
	++(*pos);

	return (void *) pos;
}

static int sequence_show(struct seq_file *s, void *v)
{
	loff_t *spos = (loff_t *) v;
	seq_printf(s, "%Ld\n", *spos);

	return 0;
}

static void sequence_stop(struct seq_file *s, void *v)
{
}

static struct seq_operations sequence_sops = {
	.start	= sequence_start,
	.next	= sequence_next,
	.show	= sequence_show,
	.stop	= sequence_stop,
};

int sequence_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &sequence_sops);
}

static const struct file_operations sequence_fops = {
	.open		= sequence_open,
	.read		= seq_read,
	.release	= seq_release,
	.llseek		= seq_lseek,
};

static int __init sequence_init(void)
{
	debugfs_file = debugfs_create_file("sequence", 0400, NULL, NULL,
						&sequence_fops);
	return 0;
}

static void __exit sequence_exit(void)
{
	debugfs_remove(debugfs_file);
}

module_init(sequence_init);
module_exit(sequence_exit);

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");
