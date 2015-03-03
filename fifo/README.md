
NAME
----

fifo - First in first out (FIFO) driver.

DESCRIPTION
-----------

This fifo driver allows data to be written and then read out.
The first data written will be the first data read.

To test this driver first build it and install it in to the kernel.

    cd fifo_rw/
    make
    sudo insmod fifo.ko
 
Then build the test programs, which are simple utilities for writing and
reading integers.

    cd test/
    make

Now numbers can be written and then read back out.

*Note* sudo may be required depending on the permissions of /dev/fifo0.

    cd test/
    ./fifow 5
    ./fifow 7
    ./fifor 1  # (read 1 number)
    5
    ./fifor 1
    7
    ./fifor 1

The `fifo_rw` driver has a problem, it does not take in to account
concurrency issues.  In this simple usage case there is not enough going
on at once for a race condition to create problems.

The `fifo_xxx` driver modifies the `fifo_rw` to exaggerate race
conditions.

    (make sure the previous fifo driver is unloaded)
    sudo rmmod fifo

    cd fifo_xxx/
    make
    sudo insmod fifo.ko

Testing the `fifo_xxx` driver requires two terminals.  The first write
will block waiting for a second write before proceeding (and creating a
race condition).

    (Terminal 1)
    cd test/
    ./fifow 1
    ./fifow 3
    ./fifor
    1
    ./fifor
    3
    
    (Terminal 2)
    cd test/
    ./fifow 2
    ./fifow 4
    ./fifor
    4
    ./fifor
    (nothing ???)

Strange things will happen with this driver.  Multiple reads/writes may
be required to get it to proceed.  And output data may be lost or in the
wrong order.  Segfaults may also appear in the logs.

To fix these race conditions mutexes are added which is shown by the
`fifo_fix` driver.

    cd fifo_fix/
    make
    sudo insmod fifo.ko

    (Terminal 1)
    cd test/
    ./fifow 1
    ./fifow 3
    ./fifor
    1
    ./fifor
    3
    
    (Terminal 2)
    cd test/
    ./fifow 2
    ./fifow 4
    ./fifor
    2
    ./fifor
    4

As another variation, the `fifo_sysfs` driver sysfs files which allow
the read and write offsets to be read.

    cat /sys/class/fifo/fifo0/write_offset
    0
    cat /sys/class/fifo/fifo0/read_offset
    0

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>
