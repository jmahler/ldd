#!/bin/sh

if [ -z "$1" ]; then
	cat <<-EOF
		  usage: $0 <device>
		    <device> := "/sys/kernel/debug/foo_debugfs/foo" |
		                "/sys/kernel/foo_sysfs/foo" |
		                ...
	EOF
	exit 1
fi

DEV=$1

set -vx

# store services in foo
sudo dd if=/etc/services of=$DEV bs=4096 count=1
grep ssh $DEV

