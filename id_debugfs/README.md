
NAME
----

debugfs_id - A file with an id using debugfs.

DESCRIPTION
-----------

Creates a file in debugfs representing an id.  The id can be read out.
And if the correct id is written, the write will succeed, otherwise it
will fail.

	DEV="/sys/kernel/debug/dbgid"

	cat $DEV/id >id.txt

	cp id.txt id-bad.txt
	sed -i 's/a/X/g' id-bad.txt

	cat id.txt >$DEV/id
	echo $?
	0

	cat id-bad.txt >$DEV/id
	cat: write error: Invalid argument
	echo $?
	1

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

