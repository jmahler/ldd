
NAME
----

hello - Hello world kernel module

DESCRIPTION
-----------

The `hello` module simply prints a message when it is loaded/unloaded.

    hello$ make
    (should compile without error and produce hello.ko)
    hello$ sudo insmod hello.ko
     Hello, World
    hello$ sudo rmmod hello
     Goodbye, cruel world

For more information refer to the main documentation (`doc/`);

SYSTEM MODULE
-------------

This module can also be added to the system so it can be loaded automatically.

The first step is to place the module where it can be found.

    $ cp hello.ko /lib/modules/3.15.0-rc7-00058-gf2159d1/kernel/drivers/char/

Then run `depmod` to update modules.dep and modules.dep.bin.
These files can be thought of as a database of the available drivers.

    $ depmod

To verify that it worked, run modinfo.

	$ modinfo
    filename:       /lib/modules/3.15.0-rc7-00058-gf2159d1/kernel/drivers/char/hello.ko
    license:        GPL
    author:         Jeremiah Mahler <jmmahler@gmail.com>
    depends:        
    vermagic:       3.15.0-rc7-00058-gf2159d1 SMP mod_unload modversions 

Now modprobe can be used to load the module instead of insmod

    $ modprobe hello

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

