
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>

int main(int argc, char **argv) {

	char dev_file[] = "/dev/fifo0";
	char buf[10];
	int devfd;
	int i;
	int n;
	int N = 400;
	//int FIFO_SIZE = 8192;
	int FIFO_SIZE = 5000;

	struct timeval tv_start, tv_end;
	int usec;
	float sec;
	int bytes;
	float Mbytes;


	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	/*
	 * Read and write the fifo many times and keep track
	 * of how long it took as well as the number of bytes
	 * that were transferred.
	 * fifo size is 3
	 */

	buf[0] = 5;
	buf[1] = 6;
	buf[2] = 7;

	gettimeofday(&tv_start, NULL);

	for (i = 0; i < N; i++) {

		n = write(devfd, buf, FIFO_SIZE);
		if (n != FIFO_SIZE) {
			printf("should have wrote %d values, but it wrote %d\n", 3, n);
			return 1;
		}

		n = read(devfd, buf, FIFO_SIZE);
		if (n != FIFO_SIZE) {
			printf("should have read %d values, but it wrote %d\n", 3, n);
			return 1;
		}
	}
	gettimeofday(&tv_end, NULL);

	usec = tv_end.tv_usec - tv_start.tv_usec;
	sec = usec * 1e-6;
	bytes = (FIFO_SIZE*2)*(N+1);
	Mbytes = bytes / 1e6;
	printf("bytes: %d\n", bytes);
	printf("usec: %d\n", usec);
	printf("sec: %f\n", sec);

	printf("transfer rate: %f MB/sec\n", (float) Mbytes/sec);

	return 0;  // OK
}
