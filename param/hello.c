
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static int howmany = 1;
module_param(howmany, int, S_IRUGO);
// This will also create an entry in /sys/module/hello/parameters/howmany

static int __init hello_init(void)
{
	int i;

	for (i = 0; i < howmany; i++) {
		printk(KERN_ALERT "Hello, World\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	int i;

	for (i = 0; i < howmany; i++) {
		printk(KERN_ALERT "Goodbye, cruel world\n");
	}
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
