
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEVFILE "/dev/data0"
#define MAX_DATA 128

int main() {

	int devfd;
	char bufw[MAX_DATA];
	char bufr[MAX_DATA];
	int i, j;
	ssize_t ret;
	off_t ofs;
	int testn = 0;

	devfd = open(DEVFILE, O_RDWR);
	if (-1 == devfd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	/* store count values */

	for (i = 0; i < MAX_DATA; i++) {
		bufw[i] = i;
	}

	ret = write(devfd, (void *) bufw, MAX_DATA);
	if (-1 == ret) {
		perror("write()");
		return EXIT_FAILURE;
	}

	/* test 1, read fro the begining */

	ofs = lseek(devfd, 0, SEEK_SET);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	ret = read(devfd, bufr, MAX_DATA);
	if (-1 == ret) {
		perror("read()");
		return EXIT_FAILURE;
	}
	for (i = 0; i < MAX_DATA; i++) {
		if (bufr[i] != bufw[i]) {
			printf("[%u] invalid: %u != %u\n", i, bufr[i], bufw[i]);
			return EXIT_FAILURE;
		}
	}
	printf("test %u OK\n", ++testn);

	/* test 2, read from the end */

	ofs = lseek(devfd, -1, SEEK_END);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	ret = read(devfd, bufr, MAX_DATA);
	if (-1 == ret) {
		perror("read()");
		return EXIT_FAILURE;
	}
	for (i = 0, j = MAX_DATA-1; i < MAX_DATA; i++) {
		if (bufr[i] != bufw[j]) {
			printf("invalid: r[%u] = %u, w[%u] = %u\n", i, bufr[i], j, bufw[j]);
			return EXIT_FAILURE;
		}

		if (++j > MAX_DATA-1)
			j = 0;
	}
	printf("test %u OK\n", ++testn);

	/* test 3, start from the middle */

	ofs = lseek(devfd, 0, SEEK_SET);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	ofs = lseek(devfd, 64, SEEK_CUR);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	ret = read(devfd, bufr, MAX_DATA);
	if (-1 == ret) {
		perror("read()");
		return EXIT_FAILURE;
	}
	for (i = 0, j = 64; i < MAX_DATA; i++) {
		if (bufr[i] != bufw[j]) {
			printf("invalid: r[%u] = %u, w[%u] = %u\n", i, bufr[i], j, bufw[j]);
			return EXIT_FAILURE;
		}

		if (++j > MAX_DATA-1)
			j = 0;
	}
	printf("test %u OK\n", ++testn);

	close(devfd);

	return EXIT_SUCCESS;
}
