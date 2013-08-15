obj-m := hello.o
KVERSION := $(shell uname -r)

all:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

diff: hello.c
	diff -u ../hello/hello.c hello.c > diff; true

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	-rm -f diff
