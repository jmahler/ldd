
obj-m := sysx.o

path := $(shell uname -r)

all:
	make -C /lib/modules/$(path)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean

