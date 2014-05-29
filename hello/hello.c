#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk(KERN_DEBUG "Hello, World\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_DEBUG "Goodbye, cruel world\n");
}

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");

module_init(hello_init);
module_exit(hello_exit);
