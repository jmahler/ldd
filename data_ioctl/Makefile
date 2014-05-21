
ifneq ($(KERNELRELEASE),)

obj-m := data.o

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

diff:
	diff -u ../data_chr/data.c data.c > diff; true

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	-rm -f diff

endif

