
NAME
----

faulty - faulty module with creates a segfault during a read or write

DESCRIPTION
-----------

This module can be used to create a segfault which will generate a
backtrace in the logs.  This can be useful when learning how to
understand backtraces.  Since it is know where the fault is in the code,
the task is to determine how the backtrace points to this location.

Be sure to have `CONFIG_KALLSYSM` enabled in the kernel.  Otherwise
the backtrace will be even more incomprehensible.

Read Fault
==========

First, build and load the fault module.

    make
    sudo insmod faulty.ko

Then try to read from the faulty device.

    sudo dd if=/dev/faulty of=/dev/null count=40

A large backtrace will be produced in the logs.

    ...
    Mar 11 00:29:59 frost kernel: Stack:
    Mar 11 00:29:59 frost kernel:  abababababababab ffff880018a2bd00 ffff880018a2bd00 0000000000d77000
    Mar 11 00:29:59 frost kernel:  0000000000000001 0000000000000000 ffffffff811ae0c2 00007fff25c12932
    Mar 11 00:29:59 frost kernel:  0000000000000000 00007fff25c11fe0 0000000000000000 00007fff25c11fe0
    Mar 11 00:29:59 frost kernel: Call Trace:
    Mar 11 00:29:59 frost kernel:  [<ffffffff811ae0c2>] ? SyS_read+0x42/0xb0
    Mar 11 00:29:59 frost kernel:  [<ffffffff8140fb6d>] ? system_call_fastpath+0x16/0x1b
    Mar 11 00:29:59 frost kernel: Code: 0c 48 89 44 24 0c 89 da 48 89 44 24 14 c7 44 24 1c ab ab ab ab e8 5e 7f 1c e1 85 c0 48 63 d0 48 89 d8 48 0f 45 c2 48 83 c4 10 5b <c3> 50 8b 35 c5 21 00 00 48 8b 3d b6 21 00 00 e8 19 15 27 e1 48 
    Mar 11 00:29:59 frost kernel: RIP  [<ffffffffa00b90a3>] faulty_read+0x53/0x54 [faulty]
    ...

First, notice that the `RIP` line indicates that the error occurred in
the `[faulty]` module in the `faulty_read` function.

The next thing to notice is that the pattern which was being written with
`memset()` appears on the stack trace.  A unique data pattern is not always
available, but it is an interesting characteristic nonetheless.

    /* Create a buffer overflow fault */
    memset(stack_buf, 0xab, 20);
    
There is much more information in the back trace but in this case this
would likely be enough to find the fault.

AUTHOR
------
Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>
