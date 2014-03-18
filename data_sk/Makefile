
obj-m := data.o

path := $(shell uname -r)

all:
	make -C /lib/modules/$(path)/build M=$(PWD) modules

diff: data.c
	diff -u ../data_rw/data.c data.c > diff; true

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean
	-rm -f diff
