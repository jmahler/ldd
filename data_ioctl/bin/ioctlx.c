
/*
 * Read and write values to the ioctlx driver using ioctl().
 *
 *   $ sudo ./ioctlx
 *   (reads current value)
 *
 *   $ sudo ./ioctlx 123
 *   (set value to 123)
 *   $ sudo ./ioctlx
 *   read: 123
 *
 *   $ sudo ./ioctlx 0
 *   (reset to zero)
 */

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEVFILE "/dev/ioctlx0"

#define IOCTLX_IOC_MAGIC 'm'

#define IOCTLX_IOCRESET _IO(IOCTLX_IOC_MAGIC,  1)
#define IOCTLX_IOCWX    _IOW(IOCTLX_IOC_MAGIC, 2, int)
#define IOCTLX_IOCRX    _IOR(IOCTLX_IOC_MAGIC, 3, int)

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
			ioctl(devfd, IOCTLX_IOCRESET);
		} else {
			ioctl(devfd, IOCTLX_IOCWX, &x);
		}

		printf("wrote: %d\n", x);
	} else {
		/* read */
		ioctl(devfd, IOCTLX_IOCRX, &x);

		printf("read: %d\n", x);
	}

	close(devfd);

	return 0;
}
