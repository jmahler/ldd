
ifneq ($(KERNELRELEASE),)

obj-m := linked_list.o
ccflags-y += -DDEBUG

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif

