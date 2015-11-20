
ifneq ($(KERNELRELEASE),)

ccflags-y += -DDEBUG
obj-m := usb_notify.o

else

KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$(PWD)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
