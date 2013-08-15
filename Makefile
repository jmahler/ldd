
all: diff

diff:
	cd param; make diff
	cd data_rw; make diff
	cd data_sk; make diff
	cd data_ioctl; make diff
	cd fifo_rw; make diff
	cd fifo_sysfs; make diff
	cd fifo_xxx; make diff

clean:
	cd param; make diff
	cd data_rw; make diff
	cd data_sk; make diff
	cd data_ioctl; make diff
	cd fifo_rw; make clean
	cd fifo_sysfs; make clean
	cd fifo_xxx; make clean
