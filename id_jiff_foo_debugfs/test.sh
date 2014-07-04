#!/bin/sh

set -v

DEV="/sys/kernel/debug/id_jiff_foo_debugfs"

cat $DEV/id >id.txt

# correct id
cat id.txt >$DEV/id
echo $?

# bad id with correct length
cp id.txt id-bad.txt
sed -i 's/a/X/g' id-bad.txt
#
cat id-bad.txt >$DEV/id
echo $?

# bad id with wrong length
cp id.txt id-bad2.txt
cat id.txt >> id-bad2.txt
#
cat id-bad2.txt >$DEV/id
echo $?

# store some foo
sudo dd if=/etc/services of=$DEV/foo
grep ssh $DEV/foo

rm -f id.txt id-bad.txt id-bad2.txt

