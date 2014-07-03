## Name

ldd - Linux device driver examples

## Description

This is a collection of Linux device driver examples that were collected
while learning how to write drivers.  Some are unique to this project
and some have been derived from examples given in books [[1]].

In some examples, such as data_ and fifo_, the driver is built up in
stages with each stage only adding a single new feature.  The `diff`
program can then be used to see what has changed compared to the
previous version.

## Index

  - hello - "Hello, World!" message when driver loaded (insmod).

  - hello-usb-keyboard - "Hello, World!" message when usb keyboard plugged in.

  - debugfs_id - An id file using debugfs which can be read or written.
	If the correct id is written the write succeeds, otherwise it fails.

  - hello-debugfs - read/write an id, read jiffies, and store foo.

  - id-misc - An id file using a misc driver, which is a simplified
	version of a char driver.  If the correct id is written the write
	succeeds, otherwise it fails.

  - data_ - A data (ram) driver.

	- data_chr - Basic character driver infrastructure.

	- data_rw - Addition of read/write operations.

	- data_sk - Addition of seek operation.

	- data_ioctl - Addition of ioctl() support.

  - zero - /dev/zero driver.

  - null - /dev/null driver.

  - fifo_ - A first in first out (FIFO) driver.

	- fifo_rw - Read/write FIFO operations.

	- fifo_xxx - Shows how race conditions break the current
	  implementation.

	- fifo_fix - Fix the race conditions using mutexes.

	- fifo_sysfs - Add Sysfs support so state of FIFOs can be read.

## References

  [1] [Linux Device Drivers][1]
  [1]: http://lwn.net/Kernel/LDD3/

## Author

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

## Copyright

Copyright &copy; 2014, Jeremiah Mahler All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

  [gpl]: http://www.gnu.org/licenses/gpl.html

