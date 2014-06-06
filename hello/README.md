
NAME
----

hello - Hello world kernel module

DESCRIPTION
-----------

The `hello` module simply prints a message when it is loaded.

(terminal 1)
    
    jeri@hudson:~$ sudo tail -n 0 -f /var/log/kern.log
    Jun  6 14:41:53 hudson kernel: [67076.079221] Hello, World
    Jun  6 14:41:59 hudson kernel: [67082.496149] Goodbye, cruel world
    ^Cjeri@hudson:~$

(terminal 2)
    
    jeri@hudson:~/ldd/hello$ make
    make -C /lib/modules/`uname -r`/build M=/home/jeri/ldd/hello
    make[1]: Entering directory '/home/jeri/linux'
      CC [M]  /home/jeri/ldd/hello/hello.o
      Building modules, stage 2.
      MODPOST 1 modules
      CC      /home/jeri/ldd/hello/hello.mod.o
      LD [M]  /home/jeri/ldd/hello/hello.ko
    make[1]: Leaving directory '/home/jeri/linux'
    jeri@hudson:~/ldd/hello$ sudo insmod hello.ko
    jeri@hudson:~/ldd/hello$ sudo rmmod hello
    
SYSTEM MODULE
-------------

This module can also be added to the system so it can be loaded automatically.

The first step is to place the module where it can be found.

    $ cp hello.ko /lib/modules/3.15.0-rc7-00058-gf2159d1/extra/

Then run `depmod` to update modules.dep and modules.dep.bin.
These files can be thought of as a database of the available drivers.

    $ depmod

To verify that it worked, run modinfo.

	$ modinfo
    filename:       /lib/modules/3.15.0-rc7-00058-gf2159d1/extra/hello.ko
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

COPYRIGHT
---------

Copyright &copy; 2014, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

