#!/bin/sh

set -v

DEV="/sys/kernel/debug/hello"

cat $DEV/id >id.txt

cp id.txt id-bad.txt
sed -i 's/a/X/g' id-bad.txt

cat id.txt >$DEV/id
echo $?

cat id-bad.txt >$DEV/id
echo $?

sudo dd if=/etc/services of=$DEV/foo
grep ssh $DEV/foo
