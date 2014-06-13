
NAME
----

hello-debugfs - Hello world for debugfs.

DESCRIPTION
-----------

Shows how to create files and directories using debugfs.  An id can be
read from the file.  If the id written back to the file it will return
success, otherwise it will return failure.

    #!/bin/sh
    
    set -v
    
    ID="/sys/kernel/debug/hello/id"
    
    cat $ID >id.txt
    
    cp id.txt id-bad.txt
    sed -i 's/a/X/g' id-bad.txt
    
    cat id.txt >$ID
    echo $?
    1  # failure

    
    cat id-bad.txt >$ID
    echo $?
    0  # OK

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

