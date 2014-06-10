#!/bin/sh

set -v

# read the id
dd if=/dev/misc of=id.txt bs=50 count=1
cat id.txt && echo ""

# write the id, valid
dd if=id.txt of=/dev/misc bs=50 count=1
echo $?

# write part of the id, invalid
dd if=id.txt of=/dev/misc bs=2 count=1
echo $?

