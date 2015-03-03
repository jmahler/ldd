
ifneq ($(KERNELRELEASE),)

obj-m := fifo.o

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

diff: fifo.c
	diff -u ../data_rw/data.c fifo.c > diff; true

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f diff

endif

