#!/bin/sh

if [ -z "$1" ]; then
	cat <<-EOF
		  usage: $0 <device>
		    <device> := "sys/kernel/debug/jiff_debugfs/jiffies" | ...
	EOF
	exit 1
fi

DEV=$1

set -vx

cat $DEV
sleep 1;
cat $DEV
# (should be different)

