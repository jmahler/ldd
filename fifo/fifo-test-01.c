
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

int main(int argc, char **argv) {

	char dev_file[] = "/dev/fifo0";
	char buf[10];
	int devfd;
	int i;
	int n;

	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	// the fifo size is 3 

	// {{{ complete fill and empty

	// {{{ one by one
	for (i = 0; i < 5; i++) {
		buf[0] = i;
		write(devfd, buf, 1);
	}

	for (i = 0; i < 3; i++) {
		read(devfd, buf, 1);
		if (buf[0] != i) {
			printf("incorrect value read at offset %d\n", i);
			return 1;  // ERROR
		}
	}
	// }}}

	// {{{ whole write

	buf[0] = 5;
	buf[1] = 6;
	buf[2] = 7;

	n = write(devfd, buf, 3);
	if (n != 3) {
		printf("should have wrote %d values, but it wrote %d\n", 3, n);
		return 1;
	}

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;

	n = read(devfd, buf, 3);
	if (n != 3) {
		printf("should have read %d values, but it wrote %d\n", 3, n);
		return 1;
	}

	for (i = 0; i < 3; i++) {

		if (buf[i] != i + 5) {
			printf("whole write, incorrect value read at offset %d\n", i);
			printf("  got %d was expecting %d\n", buf[i], i + 5);
			return 1;  // ERROR
		}
	}
	// }}}

	// }}}

	printf("OK\n");

	return 0;  // OK
}
