#!/bin/sh

DEV="/sys/kernel/debug/foo_debugfs"

set -vx

# store some foo
sudo dd if=/etc/services of=$DEV/foo bs=4096 count=1
grep ssh $DEV/foo

