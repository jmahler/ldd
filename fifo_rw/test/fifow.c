/*
 * Write numbers given on the command line to the fifo.
 *
 *   $ ./fifow 12 25 16
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
	int i;
	char n;
	int devfd;
	ssize_t nr;

	devfd = open(DEVFILE, O_WRONLY);
	if (-1 == n) {
		perror("open()");
		return EXIT_FAILURE;
	}

	for (i = 1; i < argc; i++) {
		n = atoi(argv[i]);
		nr = write(devfd, &n, sizeof(n));

		if (-1 == nr) {
			perror("write()");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
