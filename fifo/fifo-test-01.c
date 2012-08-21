
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define FIFO_SIZE 8192

// {{{ _read/_write utils
void _read(const int devfd, const int size, char *check_buf)
{
	int i;
	char *buf;
	char *buf_start;

	buf = malloc(size);
	buf_start = buf;

	read(devfd, buf, size);

	for (i = 0; i < size; i++, check_buf++, buf++) {
		if (*check_buf != *buf)
			printf("_read: not equal [%d]: %d != %d\n",
					i, *check_buf, *buf);
	}

	free(buf_start);
}

void _write(const int devfd, const int size, char *set_buf)
{
	write(devfd, set_buf, size);
}
// }}}

int main(int argc, char **argv) {

	char dev_file[] = "/dev/fifo0";
	int devfd;
	int i;

	if ( (devfd = open(dev_file, O_RDWR)) < 0) {
		printf("Unable to open device '%s': %s\n",
				dev_file, strerror(errno));

		return 1;  // ERROR
	}

	// {{{ write, write, read, write
	{
	int sz = 4005;
	char *check_buf1 = malloc(sz);
	char *cptr = check_buf1;
	for (i = 0; i < sz; i++, cptr++) {
		*cptr = i;	
	}

	_write(devfd, sz, check_buf1);
	_write(devfd, sz, check_buf1);
	_read(devfd, sz, check_buf1);
	_write(devfd, sz, check_buf1);
	_read(devfd, sz, check_buf1);
	_read(devfd, sz, check_buf1);

	free(check_buf1);
	}
	// }}}

	// {{{ write overflow

	{
	int sz = FIFO_SIZE;
	char *check_buf1 = malloc(sz);
	char *cptr = check_buf1;
	for (i = 0; i < sz; i++, cptr++) {
		*cptr = i;	
	}

	_write(devfd, sz, check_buf1);  // should be full
	_write(devfd, sz, check_buf1);  // all will be lost
	_read(devfd, sz, check_buf1);
	// TODO - should be empty

	free(check_buf1);
	}
	// }}}

	printf("OK\n");

	return 0;  // OK
}
