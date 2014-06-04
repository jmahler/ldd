#!/bin/sh

set -v

id=`cat /dev/misc`
echo "$id"

echo -n $id > /dev/misc
echo $?

echo -n "WRONG" > /dev/misc
echo $?

dd if=/dev/misc of=out.txt bs=1 count=20
cat out.txt; echo ""

dd if=/dev/misc of=out.txt bs=2 count=10
cat out.txt; echo ""
