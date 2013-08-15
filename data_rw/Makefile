
obj-m := data.o

path := $(shell uname -r)

all:
	echo $(EXTRA_CFLGAS)
	make -C /lib/modules/$(path)/build M=$(PWD) modules

diff: data.c
	diff -u ../data_chr/data.c data.c > diff; true

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean
	-rm -f diff
