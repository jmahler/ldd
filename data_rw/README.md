
NAME
----

data_rw - read/write char device operations

SYNOPSIS
--------

This is a character driver that supports read/write of
a fixed buffer of data.  If the read/write length exceeds
the maximum (MAX_DATA = 128) it will loop around.

It can be tested using 'dd' if the length is less than
the maximum.  If the length exceeds the maximum the
looping behaviour will make it interesting.

  $ sudo dd if=data.c of=/dev/data0 bs=128 count=1

  $ sudo dd if=/dev/data0 of=out1 bs=128 count=1
  (out1 should have the data from data.c)

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<https://plus.google.com/101159326398579740638/about>

