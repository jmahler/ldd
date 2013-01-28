
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
	int n;

	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	for (i = 0; i < N; i++) {
		buf[i] = i + 5;
	}

	n = write(devfd, buf, N);
	if (n != N) {
		printf("short write, %d values out of %d\n", n, N);
	}

	return 0;  // OK
}
