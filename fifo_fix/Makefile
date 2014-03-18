
obj-m := fifo.o

path := $(shell uname -r)

all:
	make -C /lib/modules/$(path)/build M=$(PWD) modules

diff: fifo.c
	diff -u ../fifo_xxx/fifo.c fifo.c > diff; true

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean
	-rm -f diff
