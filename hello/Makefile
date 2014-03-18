
#
# For a description of how this Makefile works refer to
# Chapter 2 of Jonathan Corbet's book, Linux Device Drivers (2005).
#

obj-m := hello.o

path := $(shell uname -r)

all:
	make -C /lib/modules/$(path)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(path)/build M=$(PWD) clean

