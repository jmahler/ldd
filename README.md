## NAME

ldd - Linux Device Driver examples

## DESCRIPTION

This is a collection Linux device drivers examples.

Many of the examples were derived from two Linux device
driver books: Linux Device Drivers (2005) and
Essential Linux Device Drivers (2008).

## INDEX

(in order of increasing complexity)

  - hello - "Hello, World" driver, prints a message on load/unload

  - param - "Hello, World" that takes a parameter

The "data" driver simply provides memory that can be read/written.
The driver is built up in steps.  What has changed between steps
can be seen using 'diff'.

    $ cd data_rw
    $ diff data.c ../data_chr/data.c

  - data\_chr - creation of a character device, load/unload

  - data\_rw - read/write

  - data\_sk - seekable read/write

Instead of reading/writing a device file, ioctl() can be used
for a single value.

  - ioctlx - read/write value using ioctl()

From the character driver techniques discussed so far,
the familiar `/dev/null` and `/dev/zero` can be constructed.

  - null - creating /dev/null from a char device

  - zero - creating /dev/zero from a char device

A fifo (first in first out) driver exemplifying poll() operations.

  - fifo\_rw - read/write fifo, similar to data\_rw

To try out the fifo, use echo and cat.

    $ echo -n "Hello, World" > /dev/fifo0
    $ cat /dev/fifo0
    (should see the stored message)

  - fifo\_poll - addition of poll() capability

## REQUIRED PACKAGES

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

## AUTHOR

Jeremiah Mahler <jmmahler@gmail.com><br>
<https://plus.google.com/101159326398579740638/about>

## COPYRIGHT

Copyright &copy; 2013, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

