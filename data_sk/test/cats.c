
/*
 * Works just like 'cat' but can seek before
 * producing output.
 *
 * Seek to byte 100 then cat the reset of 'file.txt'
 *
 *   $ cats file.txt 100
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_DATA 1024

int main(int argc, char* argv[])
{
	int fd;
	char buf[MAX_DATA];
	ssize_t ret;
	char *devfile;
	char *set_end;
	off_t ofs;
	char *usage = "usage: cats <in file> <SET|END> <seek bytes>";
	int whence = 0;

	if (argc != 4) {
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}

	devfile = argv[1];
	set_end = argv[2];
	ofs = atoi(argv[3]);

	fd = open(devfile, O_RDONLY);
	if (-1 == fd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	if (0 == strcmp(set_end, "SET")) {
		whence = SEEK_SET;
	} else if (0 == strcmp(set_end, "END")) {
		whence = SEEK_END;
	} else {
		fprintf(stderr, "unknown seek type '%s'\n", set_end);
		fprintf(stderr, "%s\n", usage);
		exit(1);
	}

	//printf("seek_bytes: %jd\n", (intmax_t) ofs);

	ofs = lseek(fd, ofs, whence);
	if ((off_t) -1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	while ( (ret = read(fd, buf, MAX_DATA))) {
		if (errno == EINTR) {
			continue;
		} else if (-1 == ret) {
			perror("read()");
			return EXIT_FAILURE;
		}

		write(STDOUT_FILENO, buf, ret);
	}
		
	close(fd);

	return EXIT_SUCCESS;
}
