# NAME

ldd - Linux Device Driver examples

# DESCRIPTION

This is a collection Linux device drivers examples.

Many of the examples are derived from two Linux device
driver books: Linux Device Drivers (2005) and
Essential Linux Device Drivers (2008).

# INDEX

(in order from simple to more complex)

  - hello - "Hello, World" driver, prints a message on load/unload

  - param - hello but with a parameter

  - datchr - character driver creation

  - datrw - read/write

  - datsk - seekable read/write

	(to be developed)

  - datsem - semaphore controlled data

  - ioctl - out of band data read/write using ioctl

  - readv, writev

  - mmap

## Required Packages

To compile these modules under Debian the following
packages had to be installed.  Adjust for your specific
kernel version.

    root# apt-get install linux-source-3.9
    root# apt-get install linux-headers-3.9-1-686-pae

## Compiling Modules

Each of the examples will have a Makefile similar to
the one given below except with the name of that
specific module in place of 'example.o'.


    obj-m := example.o
    
    path := $(shell uname -r)
    
    all:
        echo $(EXTRA_CFLAGS)
        make -C /lib/modules/$(path)/build M=$(PWD) modules
    
    clean:
        make -C /lib/modules/$(path)/build M=$(PWD) clean

With this Makefile typing 'make' should build the module.
And then root can insmod and rmmod the module as usual.

    root# insmod example.ko
    root# rmmod example

# COPYRIGHT

Copyright &copy; 2013, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

