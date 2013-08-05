## Name

ldd - Linux Device Driver examples

## Description

There are two main books about Linux device drivers:

 * [Linux Device Drivers][ldd] by Jonathan Corbet, et.al.
 * [Essential Linux Device Drivers][eldd] by Sreekrishnan Venkateswaran.

The books are excellent and anyone working on Linux drivers should
have them both.
But the examples in these books have,
at least in this authors experience, some shortcomings.
Some examples were too conceptual and others were
not very concise.  And some examples introduced too many concepts
at once.

The driver examples in this project are meant to be simple and concise.
It is however unavoidable that a driver of any significance
will contain a non-trivial amount of code.
To keep each driver conceptually simple each one is built
in steps.  Each step is a working driver, albeit with limited functionality.
And the differences between each step can be easily seen
using the `diff` command.

	$ cd fifo_xxx
	$ diff fifo.c ../fifo_rw/fifo.c

  [ldd]: http://lwn.net/Kernel/LDD3/  "Linux Device Drivers"
  [eldd]: http://elinuxdd.com/  "Essential Linux Device Drives"

The following is an overview of the drivers in this project:

 - [Hello, World](#hello-world)
  - hello - "Hello, World" driver, prints a message on load/unload
  - param - takes a parameter, number of message to print
 - [Read/Write Data](#readwrite-data)
  - data\_chr - character device create (load/unload)
  - data\_rw - read/write
  - data\_sk - seek-able read/write
  - ioctlx - read/write value using ioctl()
  - null - /dev/null char device
  - zero - /dev/zero char device
 - [Sysfs](#sysfs)
  - sysx\_file - read/write a single file (attribute) in sysfs
  - sysx\_file2 - read/write a two files in sysfs
  - sysx\_group - equivalent of sysx\_file2 using `sysfs_create_group()`.
  - sysx\_ktype - use kobj\_ktype to define default sysfs operations
  - sysx\_ktype2 - a variation that uses `kobj_type.default_attrs`.
 - [Concurrency](#concurrency)
  - fifo\_rw - read/write fifo, similar to data\_rw
  - fifo\_xxx - create race conditions that break the fifo
  - fifo\_fix - (in development) how to fix the race conditions using mutexes

### Hello, World<a id="hello-world"></a>

The **hello** module simply prints a message on load/unload.

	hello$ sudo insmod hello.ko
	  Hello, World
	hello$ sudo rmmod hello
	  Goodbye, cruel world
	
The **param** module takes a parameter that controls the number
of times the message is printed.

	param$ sudo insmod hello.ko howmany=2
	  Hello, World
	  Hello, World

And to see what changes were need to construct the *param* driver
from the *hello* driver take the `diff`.

	~/ldd/param$ diff -u ../hello/hello.c hello.c
	--- ../hello/hello.c	2013-07-25 23:21:16.478135138 -0700
	+++ hello.c	2013-07-25 23:22:30.606131580 -0700
	@@ -1,16 +1,30 @@
	 
	 #include <linux/init.h>
	 #include <linux/module.h>
	+#include <linux/moduleparam.h>
	+
	+static int howmany = 1;
	+module_param(howmany, int, S_IRUGO);
	+// This will also create an entry in /sys/module/hello/parameters/howmany
	 
	 static int __init hello_init(void)
	 {
	-	printk(KERN_ALERT "Hello, World\n");
	+	int i;
	+
	+	for (i = 0; i < howmany; i++) {
	+		printk(KERN_ALERT "Hello, World\n");
	+	}
	+
		return 0;
	 }
	 
	 static void __exit hello_exit(void)
	 {
	-	printk(KERN_ALERT "Goodbye, cruel world\n");
	+	int i;
	+
	+	for (i = 0; i < howmany; i++) {
	+		printk(KERN_ALERT "Goodbye, cruel world\n");
	+	}
	 }
	 
	 module_init(hello_init);
	~/ldd/param$ 

### Read/Write Data<a id="readwrite-data"></a>

The *data* driver simply provides a memory region that can be
read from and written to.

The **data_chr** shows how to construct a char driver.
It doesn't do anything useful and it can only be loaded/unloaded.

The **data_rw** driver adds read and write operations.

The **data_sk** adds the seek operation.

Starting again from the **data_chr** driver, the **ioctlx**
driver shows how ioctl operations can be provided.

The **null** and **zero** drivers show how to construct
the familiar `/dev/null` and `/dev/zero` devices.
Actually the are named `/dev/null0` and `/dev/zero0` to avoid
conflicts, but they behave the same.

The **sysx\_** examples are not a char drivers but instead allow
modification of a value through a file (attribute) in /sys (sysfs).

	~/ldd/sysx_file$ sudo insmod sysx.ko
	~/ldd/sysx_file$ cat /sys/kernel/sysx/x
	0
	~/ldd/sysx_file$ echo "10" > /sys/kernel/sysx/x
	~/ldd/sysx_file$ cat /sys/kernel/sysx/x
	10

### Sysfs<a id="sysfs"></a>

Sysfs (/sys/) provides a mechanisim for accessing kernel objects
(kobject) as files.  Internal values can be read from files and
configuration options can be set by writing to these files.

The *sysx_file* example shows how to create a file in sysfs which can
read and write an integer variable in the driver.

The *sysx_file2* example expands upon the *sysx_file* example to provide
multiple variables and files.

	~/ldd/sysx_group$ sudo insmod sysx.ko
	~/ldd/sysx_group$ ls /sys/kernel/sysx/
	x  y
	~/ldd/sysx_group$ cat /sys/kernel/sysx/y
	0
	~/ldd/sysx_group$ echo "1" > /sys/kernel/sysx/y
	~/ldd/sysx_group$ cat /sys/kernel/sysx/y
	1

The *sysx_group* example shows accomplish the equivalent of *sysx_file2*
using the `sysfs_create_group()` function.

The *sysx_ktype* example shows how to define default sysfs attribute operations
using a `kobj_ktype`.

### Concurrency<a id="concurrency"></a>

The drivers up to this point have ignored the problems associated
with concurrency such as race conditions.

The **fifo_rw** driver shows how to create a FIFO which can be read
from and written to.  It does not handle concurrency but if
each read/write is completed before another is started it
will work as expected.

	~/ldd/fifo_rw$ sudo insmod fifo.ko
	~/ldd/fifo_rw$ cd test/

Several test programs are included that read and write numbers.

	~/ldd/fifo_rw/test$ sudo ./fifow 1 2
	~/ldd/fifo_rw/test$ sudo ./fifor
	1
	2

And the values that were written were correctly read out.
With the DEBUG parameter the calculated offsets in the circular
buffer used for the FIFO will be displayed.

	~/ldd/fifo_rw$ sudo insmod fifo.ko DEBUG=1

The **fifo_xxx** driver adds code which magnifies concurrency
issues by waiting for two threads to reach a given point
before proceeding.  As the processes race against each other,
share data becomes out of sync, and havoc ensues.

	~/ldd/fifo_xxx$ sudo insmod fifo.ko DEBUG=1

With this driver two processes must be run.

	~/ldd/fifo_xxx/test$ sudo ./fifow 1 &
	~/ldd/fifo_xxx/test$ sudo ./fifow 2 &
	(bad things may happen)

	~/ldd/fifo_xxx/test$ sudo ./fifor &
	~/ldd/fifo_xxx/test$ sudo ./fifor &
	(more havoc)

Various problems may happen.  The read/write pointers
may be updated twice and exceed the end of the buffer.
The data may end up in the wrong place or be overwritten.

## Required Packages

To compile these modules under Debian the following
packages had to be installed.  Adjust for your specific
kernel version.

    root# apt-get install linux-source-3.9
    root# apt-get install linux-headers-3.9-1-686-pae

### Compiling Modules

Each of the examples will have a Makefile similar to
the one given below except with the name of that
specific module in place of 'example.o'.

    obj-m := example.o
    
    path := $(shell uname -r)
    
    all:
        echo $(EXTRA\_CFLAGS)
        make -C /lib/modules/$(path)/build M=$(PWD) modules
    
    clean:
        make -C /lib/modules/$(path)/build M=$(PWD) clean

With this Makefile typing 'make' should build the module.
And then root can insmod and rmmod the module as usual.

    root# insmod example.ko
    root# rmmod example

## Author

Jeremiah Mahler <jmmahler@gmail.com><br>
<https://plus.google.com/101159326398579740638/about>

## Copyright

Copyright &copy; 2013, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

