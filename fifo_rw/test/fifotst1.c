
/*
 * Writes bytes to the fifo.
 * Then reads them back and verify that they are correct.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEVFILE "/dev/fifo0"

#define TBYTES 57

int main()
{
	int devfd;
	char wbuf[TBYTES];
	char rbuf[TBYTES];
	int i;
	ssize_t n;

	devfd = open(DEVFILE, O_RDWR);
	if (-1 == devfd) {
		perror("open()");
	}

	for (i = 0; i < TBYTES; i++) {
		wbuf[i] = i;
	}

	while (1) {
		n = write(devfd, wbuf, TBYTES);
		if (-1 == n) {
			perror("write()");
		} else if (n < TBYTES) {
			fprintf(stderr, "short write\n");
			return EXIT_FAILURE;
		}

		memset(rbuf, 0xFFFF, TBYTES);

		n = read(devfd, rbuf, TBYTES);
		if (-1 == n) {
			perror("read()");
		} else if (n < TBYTES) {
			fprintf(stderr, "short read\n");
			return EXIT_FAILURE;
		}

		for (i = 0; i < TBYTES; i++) {
			if (rbuf[i] != wbuf[i]) {
				fprintf(stderr, "wrong value at %u : %u != %u\n", i,
										rbuf[i], wbuf[i]);
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}
