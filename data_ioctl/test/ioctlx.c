#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEVFILE "/dev/data0"

#define DATA_IOC_MAGIC 'm'

#define DATA_IOCRESET _IO(DATA_IOC_MAGIC,  1)
#define DATA_IOCWX    _IOW(DATA_IOC_MAGIC, 2, int)
#define DATA_IOCRX    _IOR(DATA_IOC_MAGIC, 3, int)

int main(int argc, char* argv[])
{
	int devfd;
	int x;

	devfd = open(DEVFILE, O_RDWR);
	if (-1 == devfd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	if (2 == argc) {
		/* write */
		x = atoi(argv[1]);

		if (0 == x) {
			ioctl(devfd, DATA_IOCRESET);
		} else {
			ioctl(devfd, DATA_IOCWX, &x);
		}

		printf("wrote: %d\n", x);
	} else {
		/* read */
		ioctl(devfd, DATA_IOCRX, &x);

		printf("read: %d\n", x);
	}

	close(devfd);

	return 0;
}
