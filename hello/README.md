
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

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://github.com/jmahler>

