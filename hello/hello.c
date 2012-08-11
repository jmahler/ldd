
/*
 * https://wiki.kubuntu.org/Kernel/Dev/DKMSPackaging
 */

#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_init(void)
{
	pr_info("Hello world.\n");
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("Goodbye world.\n");
}

module_init(hello_init);
module_exit(hello_exit);

