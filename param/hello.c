
/*
 * Shows how a paramter can be given to insmod.
 *
 * # insmod hello.ko howmany=3
 *
 * Messages will be sent to syslog.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>

static int howmany = 1;
module_param(howmany, int, S_IRUGO);
// This will also create an entry in /sys/module/hello/parameters/howmany

static int __init hello_init(void)
{
	int i;

	for (i = 0; i < howmany; i++) {
		pr_info("Hello world.\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	int i;

	for (i = 0; i < howmany; i++) {
		pr_info("Goodbye world.\n");
	}
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
