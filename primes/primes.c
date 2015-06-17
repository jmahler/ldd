/*
 * NAME
 * ----
 *
 * primes - Read a sequence of prime numbers.
 *
 * DESCRIPTION
 * -----------
 *
 * Creates a debugfs file from which a sequence of prime numbers
 * can be read.
 *
 *   $ cat /sys/kernel/debug/primes
 *   2
 *   3
 *   5
 *   7
 *   11
 *   ...
 *
 * It also serves as an example of how to use the seq_file interface.
 *
 * AUTHOR
 * ------
 *
 *  Jeremiah Mahler <jmmahler@gmail.com>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

static struct dentry *debugfs_file;

static void *primes_start(struct seq_file *s, loff_t *pos)
{
	unsigned int *prime;
	unsigned int n;

	prime = kmalloc(sizeof(*prime), GFP_KERNEL);
	if (!prime)
		return NULL;

	n = 1;
	*prime = 2;
	while (n < *pos + 1) {
		bool valid_prime;
		unsigned int i;

		(*prime)++;
		valid_prime = true;
		for (i = 2; i < *prime; i++) {
			if (*prime % i == 0) {
				valid_prime = false;
				break;
			}
		}
		if (valid_prime)
			n++;
	}

	return (void *) prime;
}

static void *primes_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned int *prime = (unsigned int *) v;
	bool valid_prime;

	++(*pos);

	do {
		unsigned int i;

		(*prime)++;
		valid_prime = true;
		for (i = 2; i < *prime; i++) {
			if (*prime % i == 0) {
				valid_prime = false;
				break;
			}
		}
	} while (!valid_prime);

	return (void *) prime;
}

static int primes_show(struct seq_file *s, void *v)
{
	unsigned int *prime = (unsigned int *) v;
	seq_printf(s, "%d\n", *prime);

	return 0;
}

static void primes_stop(struct seq_file *s, void *v)
{
	kfree(v);
}

static struct seq_operations primes_sops = {
	.start	= primes_start,
	.next	= primes_next,
	.show	= primes_show,
	.stop	= primes_stop,
};

int primes_open(struct inode *inode, struct file *filp)
{
	return seq_open(filp, &primes_sops);
}

static const struct file_operations primes_fops = {
	.open		= primes_open,
	.read		= seq_read,
	.llseek		= no_llseek,
	.release	= seq_release,
};

static int __init primes_init(void)
{
	debugfs_file = debugfs_create_file("primes", 0400, NULL, NULL,
						&primes_fops);
	return 0;
}

static void __exit primes_exit(void)
{
	debugfs_remove(debugfs_file);
}

module_init(primes_init);
module_exit(primes_exit);

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");
