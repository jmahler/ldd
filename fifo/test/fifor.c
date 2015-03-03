/*
 * Read numbers from a fifo.
 *
 * Read all the numbers.
 *   $ ./fifor
 *   12
 *   13
 *   5
 *
 * Read n numbers.
 *   $ ./fifor <n>
 * 
 */

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEVFILE "/dev/fifo0"

int main(int argc, char* argv[])
{
	char n;
	int devfd;
	ssize_t nr;
	int count;

	devfd = open(DEVFILE, O_RDONLY);
	if (-1 == n) {
		perror("open()");
		return EXIT_FAILURE;
	}

	// if a count argument was given
	count = -1;  // default all
	if (2 == argc) {
		count = atoi(argv[1]);
	}

	while (count-- != 0) {
		// read 1 number
		nr = read(devfd, &n, sizeof(n));
		if (-1 == nr) {
			perror("write()");
			return EXIT_FAILURE;
		} else if (nr > 0) {
			printf("%u\n", n);
		} else {
			break;
		}
	}

	return EXIT_SUCCESS;
}
