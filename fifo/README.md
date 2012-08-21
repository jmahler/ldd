NAME
----

fifo - fifo device driver

DESCRIPTION
-----------

This is a simple first in first out (FIFO) device.
The data is read and written using a device.
And it has an upper limit after which data that is
written will be discarded.

USAGE
-----

To build the module the kernel source and kernel headers
must be present.  Then typing 'make' should build the module
and all the test programs.  The name of the resulting module
is 'fifo.ko' and it can be inserted and removed from the
kernel using standard methods (insmod, rmmod).

Once the module is inserted it will provide the device file
'/dev/fifo0' where data can be read from and written to.

TEST PROGRAMS
-------------

 - fifo-read, fifo-write - simple utilities to read or write values

 - fifo-perf - read and write a bunch of data and display performance metrics

 - fifo-test-01 - test reads and writes and make sure the values are correct

COPYRIGHT
---------

Copyright &copy; 2012, Jeremiah Mahler.  All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

 [gpl]: http://www.gnu.org/licenses/gpl.html

