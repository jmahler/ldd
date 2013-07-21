
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

void spinner() {
	static unsigned int i = 0;
	static char cs[4] = "-\\|/";

	printf("\b%c", cs[i++]);
	fflush(stdout);

	if (i >= sizeof(cs))
		i = 0;
}

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
		return EXIT_FAILURE;
	}

	for (i = 0; i < TBYTES; i++) {
		wbuf[i] = i;
	}

	while (1) {
		n = write(devfd, wbuf, TBYTES);
		if (-1 == n) {
			perror("write()");
			return EXIT_FAILURE;
		} else if (n < TBYTES) {
			fprintf(stderr, "short write\n");
			return EXIT_FAILURE;
		}

		memset(rbuf, 0xFFFF, TBYTES);

		n = read(devfd, rbuf, TBYTES);
		if (-1 == n) {
			perror("read()");
			return EXIT_FAILURE;
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

		spinner();
	}

	return EXIT_SUCCESS;
}
