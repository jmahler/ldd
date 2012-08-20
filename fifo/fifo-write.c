
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

	buf[0] = 5;
	buf[1] = 6;
	buf[2] = 7;
	write(devfd, buf, 3);

	for (i = 0; i < 3; i++) {
		printf("write: %d\n", buf[i]);
	}


	/*
	buf[0] = 5;
	write(devfd, buf, 1);
	printf("write: %d\n", buf[0]);

	buf[0] = 6;
	write(devfd, buf, 1);
	printf("write: %d\n", buf[0]);

	buf[0] = 7;
	write(devfd, buf, 1);
	printf("write: %d\n", buf[0]);
	*/

	return 0;  // OK
}
