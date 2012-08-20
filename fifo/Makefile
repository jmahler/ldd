obj-m := fifo.o
KVERSION := $(shell uname -r)

all: fifo.ko fifo-test-01 fifo-read fifo-write fifo-perf

fifo-perf: fifo-perf.c
	gcc -Wall $< -o $@

fifo-read: fifo-read.c
	gcc -Wall $< -o $@

fifo-write: fifo-write.c
	gcc -Wall $< -o $@

fifo-test-01: fifo-test-01.c
	gcc -Wall $< -o $@

fifo.ko: fifo.c
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
