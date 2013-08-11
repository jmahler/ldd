#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_DATA 128

int main(int argc, char* argv[])
{
	int devfd;
	char buf[MAX_DATA];
	ssize_t ret;
	off_t ofs;
	char *devfile;

	if (argc != 3) {
		printf("usage: %s <in file> <seek bytes>\n", argv[0]);
	}

	devfile = argv[1];
	ofs = atoi(argv[2]);

	devfd = open(devfile, O_RDONLY);
	if (-1 == devfd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	printf("seek_bytes: %jd\n", (intmax_t) ofs);

	ofs = lseek(devfd, ofs, SEEK_SET);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	while ( (ret = read(devfd, buf, MAX_DATA))) {
		if (errno == EINTR) {
			continue;
		} else if (-1 == ret) {
			perror("read()");
			return EXIT_FAILURE;
		}

		buf[ret] = '\0';

		printf("%s", buf);
	}
		
	close(devfd);

	return EXIT_SUCCESS;
}
