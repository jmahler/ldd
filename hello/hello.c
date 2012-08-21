
/*
 * https://wiki.kubuntu.org/Kernel/Dev/DKMSPackaging
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/current.h>
#include <linux/sched.h>

static int __init hello_init(void)
{
	pr_info("Hello world.\n");

	printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
			current->comm, current->pid);

	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
			current->comm, current->pid);

	pr_info("Goodbye world.\n");
}

module_init(hello_init);
module_exit(hello_exit);

