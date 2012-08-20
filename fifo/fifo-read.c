
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

	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	read(devfd, buf, 3);
	for (i = 0; i < 3; i++) {
		printf("read: %d\n", buf[i]);
	}

	/*
	read(devfd, buf, 1);
	printf("read: %d\n", buf[0]);

	read(devfd, buf, 1);
	printf("read: %d\n", buf[0]);

	read(devfd, buf, 1);
	printf("read: %d\n", buf[0]);
	*/

	return 0;  // OK
}
