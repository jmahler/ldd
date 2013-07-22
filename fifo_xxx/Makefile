
obj-m := fifo.o

path := $(shell uname -r)

all:
	echo $(EXTRA_CFLGAS)
	make -C /lib/modules/$(path)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean

