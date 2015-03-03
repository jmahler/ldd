## Name

ldd - Linux device driver examples

## Description

This is a collection of Linux device driver examples that were collected
while learning how to write drivers.  Some are unique to this project
and some have been derived from examples given in books [[1]].

In some examples, such as `data_*` and `fifo_*`, the driver is built up
in stages with each stage only adding a single new feature.  The `diff`
program can then be used to see what has changed compared to the
previous version.

## Index

  - hello - "Hello, World!" message when driver loaded (insmod).

  - hello-usb-keyboard - "Hello, World!" message when usb keyboard plugged in.

  - id_jiff_foo/

	- id_* - Read an id or write it to check if it is correct.

	- jiff_* - Read jiffies.

	- foo_* - Store some foo (a buffer of data).

	- id_jiff_foo_* - All three, id jiff and foo, in one driver.

  - data_ - A data (ram) driver.

	- data_chr - Basic character driver infrastructure.

	- data_rw - Addition of read/write operations.

	- data_sk - Addition of seek operation.

	- data_ioctl - Addition of ioctl() support.

  - zero - /dev/zero driver.

  - null - /dev/null driver.

  - fifo - A first in first out (FIFO) driver.

	- fifo_rw - Read/write FIFO operations.

	- fifo_xxx - Shows how race conditions break the current
	  implementation.

	- fifo_fix - Fix the race conditions using mutexes.

	- fifo_sysfs - Add Sysfs support so state of FIFOs can be read.

  - linked_list - Using linked list operations (linux/list.h).

  - linked_list_cache - linked_list using kmem_cache operations.

  - nf_logid - Netfilter driver which monitors all IPv4 network
	traffic and prints a message when the pattern is found.

## References

  [1] [Linux Device Drivers][1]
  [1]: http://lwn.net/Kernel/LDD3/

## Author

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

## Copyright

Copyright &copy; 2015, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

