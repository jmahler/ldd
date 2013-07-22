
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define DEVFILE "/dev/fifo0"

#define BUFSZ 128

int main(int argc, char* argv[]) {
	int devfd;
	char* msg;
	int msg_size;
	char buf[BUFSZ];
	pid_t pid;

	
	if (argc != 2) {
		fprintf(stderr, "usage: %s <msg>\n", argv[0]);
		return EXIT_FAILURE;
	}

	pid = getpid();

	msg = argv[1];
	msg_size = strlen(msg);

	assert(BUFSZ > msg_size);

	devfd = open(DEVFILE, O_RDWR);
	if (-1 == devfd) {
		perror("open()");
		return EXIT_FAILURE;
	}

	while (1) {
		printf("[%d] write '%s'\n", pid, msg);
		write(devfd, msg, msg_size);

		usleep(250e3);

		printf("[%d] read\n", pid);
		read(devfd, buf, msg_size);

		usleep(120e3);
	}

	return EXIT_SUCCESS;
}
