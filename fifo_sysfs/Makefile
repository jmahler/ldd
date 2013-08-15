
obj-m := fifo.o

path := $(shell uname -r)

all:
	echo $(EXTRA_CFLGAS)
	make -C /lib/modules/$(path)/build M=$(PWD) modules

diff: fifo.c
	diff -u ../fifo_rw/fifo.c fifo.c > diff; true

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean
	-rm -f diff
