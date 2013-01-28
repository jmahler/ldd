obj-m := scull.o
KVERSION := $(shell uname -r)

all: scull.ko

scull.ko: scull.c
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
