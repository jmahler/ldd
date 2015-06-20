/*
 * NAME
 *
 * rickroll - Rickroll .mp3 files by hijacking the open() system call.
 *
 * DESCRIPTION
 *
 * The rickroll module hijacks the open() system call so that
 * when a user attempts to open a .mp3 file, they get Rickrolled,
 * and instead open a .mp3 that they did not want to open.
 *
 *   $ sudo insmod rickroll.ko rickroll_filename=/home/user/giveyouup.mp3
 *   $ mpg321 mc_hammer-u_cant_touch_this.mp3
 *   (Never gonna give you up ...)
 *
 * This module is based on the code from Julia Evans that was used in
 * her presentation titled "You can be a kernel hacker!" [1].
 *
 * REFERENCES
 *
 *   [1]: "You can be a kernel hacker!" by Julia Evans
 *        https://www.youtube.com/watch?v=0IQlpFWTFbM
 *
 *   [2]: Rickrolling
 *        https://en.wikipedia.org/wiki/Rickrolling
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/syscalls.h>

static char *rickroll_filename;
module_param(rickroll_filename, charp, S_IRUGO);
MODULE_PARM_DESC(rickroll_filename, "Location of the rick roll .mp3 file.");

#define DISABLE_WRITE_PROTECTION (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE_WRITE_PROTECTION (write_cr0(read_cr0() | 0x10000))

asmlinkage long (*original_sys_open)(const char __user *, int, umode_t);

void **sys_call_table;

asmlinkage long rickroll_open(const char __user *filename,
				int flags, umode_t mode)
{
	int len = strlen(filename);

	if(strcmp(filename + len - 4, ".mp3") == 0) {
		mm_segment_t old_fs;
		long fd;

		pr_info("Rickrolling!");

		old_fs = get_fs();
		set_fs(KERNEL_DS);
		fd = (*original_sys_open)(rickroll_filename, flags, mode);
		set_fs(old_fs);

		return fd;
	} else {
		return (*original_sys_open)(filename, flags, mode);
	}

	return 0;
}

void **find_sys_call_table(void)
{
	unsigned long **table;

	table = (unsigned long **) PAGE_OFFSET;
	for (; table < (unsigned long **) ULONG_MAX; table++) {
		if (table[__NR_close] == (unsigned long *) sys_close) {
			return (void **) table;
		}
	}

	return NULL;
}

static int __init rickroll_init(void)
{
	if (!rickroll_filename) {
		pr_err("No .mp3 filename given.\n");
		return -EINVAL;
	}

	sys_call_table = find_sys_call_table();
	if (!sys_call_table) {
		pr_err("sys_call_table not found.\n");	
		return -EINVAL;
	}

	DISABLE_WRITE_PROTECTION;
	original_sys_open = sys_call_table[__NR_open];
	sys_call_table[__NR_open] = rickroll_open;
	ENABLE_WRITE_PROTECTION;

	pr_info("Never gonna give you up!\n");

	return 0;
}

static void __exit rickroll_exit(void)
{
	DISABLE_WRITE_PROTECTION;
	sys_call_table[__NR_open] = original_sys_open;
	ENABLE_WRITE_PROTECTION;
}

module_init(rickroll_init);
module_exit(rickroll_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler");
