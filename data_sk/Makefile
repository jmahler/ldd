
ifneq ($(KERNELRELEASE),)

obj-m := data.o

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

diff: data.c
	diff -u ../data_rw/data.c data.c > diff; true

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f diff

endif

