
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
----------

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

At this point the function inside the module where the fault occurred is known.
But the exact function within this is not known.  Using more information
from the backtrace, along with `gdb`, the fault location can be further narrowed
down.

Refer again to the RIP line (below) in the back trace.

    ...
    Mar 11 00:29:59 frost kernel: RIP  [<ffffffffa00b90a3>] faulty_read+0x53/0x54 [faulty]
    ...

The value `+0x53/0x54` indicates the symbol location and offset.  But these
values are relative to assembler code, not C code, so `gdb` must be used
to translate them.

First, start gdb with the faulty module.

    sudo gdb faulty.ko
    (gdb)

Before adding the symbol file, the location of the `.text` section in the
faulty module must be determined.  This can be found by reading its sysfs
file.

    cat /sys/module/faulty/sections/.text
    0xffffffffa0176000

Now add the symbol file from gdb.

    (gdb) add-symbol-file faulty.o 0xffffffffa0176000

Disassemble the `faulty_read` function to see locations referred to by
`+0x53/0x54` in the RIP line.

    (gdb) disassemble faulty_read
    Dump of assembler code for function faulty_read:
       0x0000000000000080 <+0>:     callq  0x85 <faulty_read+5>
       0x0000000000000085 <+5>:     push   %rbx
       0x0000000000000086 <+6>:     mov    $0x4,%ebx
       0x000000000000008b <+11>:    mov    %rsi,%rdi
       0x000000000000008e <+14>:    movabs $0xabababababababab,%rax
       0x0000000000000098 <+24>:    sub    $0x10,%rsp
       0x000000000000009c <+28>:    cmp    $0x4,%rdx
       0x00000000000000a0 <+32>:    cmovbe %rdx,%rbx
       0x00000000000000a4 <+36>:    lea    0xc(%rsp),%rsi
       0x00000000000000a9 <+41>:    mov    %rax,0xc(%rsp)
       0x00000000000000ae <+46>:    mov    %ebx,%edx
       0x00000000000000b0 <+48>:    mov    %rax,0x14(%rsp)
       0x00000000000000b5 <+53>:    movl   $0xabababab,0x1c(%rsp)
       0x00000000000000bd <+61>:    callq  0xc2 <faulty_read+66>
       0x00000000000000c2 <+66>:    test   %eax,%eax
       0x00000000000000c4 <+68>:    movslq %eax,%rdx
       0x00000000000000c7 <+71>:    mov    %rbx,%rax
       0x00000000000000ca <+74>:    cmovne %rdx,%rax
       0x00000000000000ce <+78>:    add    $0x10,%rsp
       0x00000000000000d2 <+82>:    pop    %rbx
       0x00000000000000d3 <+83>:    retq
    End of assembler dump

But this still doesn't indicate which line in the C source is at fault.
To get a listing, use the `list` command with the address at `+0x53`.
Be sure to include the `*` in the command.

    (gdb) list *0x00000000000000b5
    0xb5 is in faulty_read (/home/jeri/ldd/faulty/faulty.c:69).
    64  {
    65      int ret;
    66      char stack_buf[4];
    67
    68      /* Create a buffer overflow fault */
    69      memset(stack_buf, 0xab, 20);
    70
    71      if (count > 4)
    72      count = 4;
    73
    (gdb)

This points directly to line 69, which was the call which created
the buffer overflow.

Write Fault
-----------

First, build and load the fault module.

    make
    sudo insmod faulty.ko

Then write to the faulty device.

    sudo dd if=/dev/zero of=/dev/faulty count=20

From the backtrace, look at the RIP line.

    ...
    Mar 11 12:39:15 frost kernel: RIP  [<ffffffffa017b025>] faulty_write+0x5/0x20 [faulty]
    ...

This already narrowed it down to the right module and function.
Next, find the address of the `.text` section then use `gdb` to
interpret these values and find the exact location in the C source code.

    cat /sys/module/faulty/sections/.text
    0xffffffffa00f2000

    sudo gdb faulty.ko
    (gdb) add-symbol-file faulty.o 0xffffffffa00f2000
    add symbol table from file "faulty.o" at
        .text_addr = 0xffffffffa00f2000
    (y or n) y
    Reading symbols from faulty.o...done.
    (gdb) disassemble faulty_write
    Dump of assembler code for function faulty_write:
       0x0000000000000050 <+0>:     callq  0x55 <faulty_write+5>
       0x0000000000000055 <+5>:     movl   $0x0,0x0
       0x0000000000000060 <+16>:    xor    %eax,%eax
       0x0000000000000062 <+18>:    retq
    End of assembler dump.
    (gdb) list *0x0000000000000055
    0x55 is in faulty_write (/home/jeri/ldd/faulty/faulty.c:85).
    80
    81  static ssize_t faulty_write(struct file *filp, const char __user *buf,
    82                                          size_t count, loff_t *f_pos)
    83  {
    84          /* Create a fault by trying to de-reference a NULL pointer */
    85          *(int *)0 = 0;
    86          return 0;
    87  }
    88
    89  static int faulty_release(struct inode *inode, struct file *filp)
    (gdb)

The exact line where the fault occurred in the C source code has been found.

REFERENCES
----------

  [1]: http://www.opensourceforu.com/2011/01/understanding-a-kernel-oops/

AUTHOR
------
Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>
