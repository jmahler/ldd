#!/bin/sh

set -v

ID=/dev/misc

cat $ID >id.txt

cp id.txt id-bad.txt
sed -i 's/a/X/g' id-bad.txt

cat id.txt > $ID
echo $?

cat id-bad.txt > $ID
echo $?

