
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
	char buf[MAX_DATA];
	int i;
	ssize_t ret;
	off_t ofs;

	devfd = open(DEVFILE, O_RDWR);
	if (-1 == devfd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	for (i = 0; i < MAX_DATA; i++) {
		buf[i] = i;
	}

	ret = write(devfd, (void *) buf, MAX_DATA);
	if (-1 == ret) {
		perror("write()");
		return EXIT_FAILURE;
	}

	/* go to the middle of the buffer */
	ofs = lseek(devfd, MAX_DATA/2, SEEK_CUR);
	if (-1 == ofs) {
		perror("lseek()");
		return EXIT_FAILURE;
	}

	ret = read(devfd, buf, MAX_DATA);
	if (-1 == ret) {
		perror("read()");
		return EXIT_FAILURE;
	}

	write(STDOUT_FILENO, buf, MAX_DATA);

	close(devfd);

	return EXIT_SUCCESS;
}
