
# Check to see that the kernel source is installed

KERNEL=`uname -r`
SRC=/lib/modules/$KERNEL/build

if [ -d "$SRC" ]; then
	echo "source installed, OK";
	exit 0;
else
	echo "source not installed, ERROR";
	exit 1;
fi
