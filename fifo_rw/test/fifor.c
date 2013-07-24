/*
 * Read numbers from a fifo.
 *
 *   $ ./fifor
 *   12
 *   13
 *   5
 */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEVFILE "/dev/fifo0"

int main()
{
	char n;
	int devfd;
	ssize_t nr;

	devfd = open(DEVFILE, O_RDONLY);
	if (-1 == n) {
		perror("open()");
		return EXIT_FAILURE;
	}

	while (1) {
		nr = read(devfd, &n, sizeof(n));
		if (-1 == nr) {
			perror("write()");
			return EXIT_FAILURE;
		} else if (!(nr > 0)) {
			// read all the data
			break;
		}

		printf("%u\n", n);
	}

	return EXIT_SUCCESS;
}
