#!/bin/sh

DEV="/sys/kernel/debug/jiff_debugfs/jiffies"

set -vx

cat $DEV
sleep 1;
cat $DEV
# (should be different)

