#!/bin/sh

set -v

# read the id, large block
dd if=/dev/misc of=id.txt bs=50 count=1
cat id.txt && echo ""

# read the id, small block
dd if=/dev/misc of=id2.txt bs=1 count=50
cat id.txt && echo ""

# should have no differences
diff id.txt id2.txt

# write a valid id
dd if=id.txt of=/dev/misc bs=50 count=1
echo $?

# write an invalid id
dd if=id.txt of=/dev/misc bs=2 count=1
echo $?

