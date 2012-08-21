
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define FIFO_SIZE 8192
#define N 4000

int main(int argc, char **argv) {

	char dev_file[] = "/dev/fifo0";
	char buf[FIFO_SIZE];
	int devfd;
	int i;

	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	for (i = 0; i < N; i++) {
		buf[i] = i;
	}
	read(devfd, buf, N);

	/*
	for (i = 0; i < N; i++) {
		printf("read [%d]: %d\n", i, buf[i]);
	}
	*/

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
