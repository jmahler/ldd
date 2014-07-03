#!/bin/sh

if [ -z "$1" ]; then
	echo "usage: $0 </dev/id>";
	exit 1;
fi

set -v

DEV=$1

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

