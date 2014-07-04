#!/bin/sh

set -v

DEV="/sys/kernel/debug/id-debugfs/id"

cat $DEV >id.txt

# correct id
cat id.txt >$DEV
echo $?

# bad id with correct length
cp id.txt id-bad.txt
sed -i 's/a/X/g' id-bad.txt
#
cat id-bad.txt >$DEV
echo $?

# bad id with wrong length
cp id.txt id-bad2.txt
cat id.txt >> id-bad2.txt
#
cat id-bad2.txt >$DEV
echo $?

rm -f id.txt id-bad.txt id-bad2.txt

